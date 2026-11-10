@echo off
title STEM VCS
color 0B

:menu
cls
echo.
echo ============================================================================
echo                       STEM VCS - Version 1.0.0
echo ============================================================================
echo.
echo  [1] Install STEM (Do this first!)
echo  [2] Test Installation
echo  [3] Open Documentation
echo  [4] Open CLI Terminal
echo  [5] Exit
echo.
echo ============================================================================
echo.

choice /C 12345 /N /M "Select option (1-5): "
set CHOICE=%ERRORLEVEL%

if %CHOICE%==1 goto install
if %CHOICE%==2 goto test
if %CHOICE%==3 goto docs
if %CHOICE%==4 goto cli
if %CHOICE%==5 goto exit

:install
cls
echo.
echo ============================================================================
echo                         Installing STEM VCS
echo ============================================================================
echo.
call SETUP.bat
pause
goto menu

:test
cls
echo.
echo ============================================================================
echo                      Testing STEM Installation
echo ============================================================================
echo.
echo Checking if STEM is installed...
echo.

where stem.exe >nul 2>&1
if %ERRORLEVEL%==0 (
    echo [OK] STEM is installed and working!
    echo.
    echo ----------------------------------------------------------------------------
    stem --version
    echo ----------------------------------------------------------------------------
    echo.
    echo Installation is successful!
    echo You can now use 'stem' command from anywhere.
) else (
    echo [ERROR] STEM is NOT installed or not in PATH
    echo.
    echo Please:
    echo   1. Run Option 1 (Install STEM)
    echo   2. Close and reopen your terminal
    echo   3. Try this test again
)
echo.
echo ============================================================================
echo.
pause
goto menu

:docs
cls
echo.
echo ============================================================================
echo                     Opening Documentation
echo ============================================================================
echo.
start "" "START_HERE.md"
echo [OK] Opened START_HERE.md
echo.
timeout /t 2 >nul
goto menu

:cli
cls
echo.
echo ============================================================================
echo                      STEM VCS - Command Line
echo ============================================================================
echo.

where stem.exe >nul 2>&1
if %ERRORLEVEL%==0 (
    echo STEM is installed!
    echo.
    stem --version
    echo.
    echo ----------------------------------------------------------------------------
    echo Starting command line...
    echo ----------------------------------------------------------------------------
    echo.
    cmd /k "stem help"
) else (
    echo STEM not in PATH yet. Using local version...
    echo.
    %~dp0bin\stem.exe --version
    echo.
    echo ----------------------------------------------------------------------------
    echo Starting command line...
    echo ----------------------------------------------------------------------------
    echo.
    cmd /k "cd /d %~dp0 && echo Type: bin\stem.exe help"
)
goto menu

:exit
cls
echo.
echo ============================================================================
echo                           Thank You!
echo ============================================================================
echo.
echo STEM VCS - Own Your Code, Own Your Storage
echo.
echo For help: Read START_HERE.md
echo.
echo ============================================================================
echo.
timeout /t 2
exit
