@echo off
cls
echo.
echo ============================================================================
echo                       STEM VCS - Installation Setup
echo ============================================================================
echo.
echo This will install STEM VCS on your computer.
echo.
echo What it does:
echo   1. Adds STEM to your PATH (you can use "stem" command anywhere)
echo   2. Creates desktop shortcut
echo   3. Takes 10 seconds
echo.
echo ============================================================================
echo.
pause

echo.
echo Installing STEM VCS...
echo.

REM Get the directory where this script is located
set "STEM_DIR=%~dp0"
set "STEM_BIN=%STEM_DIR%bin"

REM Check if stem.exe exists
if not exist "%STEM_BIN%\stem.exe" (
    echo [ERROR] stem.exe not found in bin folder!
    echo Please build the project first: build_cli.bat
    pause
    exit /b 1
)

echo [1/3] Adding STEM to PATH...
setx PATH "%PATH%;%STEM_BIN%" >nul 2>&1
if %errorlevel% equ 0 (
    echo       [OK] PATH updated
) else (
    echo       [WARNING] Could not update PATH automatically
    echo       Please add manually: %STEM_BIN%
)

echo.
echo [2/3] Creating desktop shortcut...

REM Create desktop shortcut using PowerShell
powershell -Command "$WS = New-Object -ComObject WScript.Shell; $SC = $WS.CreateShortcut('%USERPROFILE%\Desktop\STEM VCS.lnk'); $SC.TargetPath = 'cmd.exe'; $SC.Arguments = '/k echo STEM VCS - Version Control System && echo. && cd /d %STEM_DIR% && .\bin\stem.exe --version && echo. && echo Type: stem help'; $SC.WorkingDirectory = '%STEM_DIR%'; $SC.IconLocation = '%SystemRoot%\System32\cmd.exe,0'; $SC.Description = 'STEM VCS Command Line'; $SC.Save()" >nul 2>&1

if %errorlevel% equ 0 (
    echo       [OK] Desktop shortcut created
) else (
    echo       [WARNING] Could not create desktop shortcut
)

echo.
echo [3/3] Creating configuration...

REM Create config directory
set "CONFIG_DIR=%USERPROFILE%\.stem"
if not exist "%CONFIG_DIR%" mkdir "%CONFIG_DIR%"

REM Create basic config file
(
echo # STEM VCS Configuration
echo # Edit this file to customize your settings
echo.
echo [user]
echo # name = Your Name
echo # email = your@email.com
echo.
echo [core]
echo editor = notepad
) > "%CONFIG_DIR%\config"

echo       [OK] Configuration created
echo.
echo.
echo ============================================================================
echo                        Installation Complete!
echo ============================================================================
echo.
echo STEM VCS has been installed successfully!
echo.
echo IMPORTANT: You MUST close this window and open a NEW terminal/PowerShell
echo            for the PATH changes to take effect.
echo.
echo After reopening terminal, test it:
echo.
echo    stem --version
echo.
echo Quick commands:
echo    stem help              - Show all commands
echo    stem init -i           - Setup a new project
echo    stem status            - Check repository status
echo.
echo For your Xylus OS project:
echo    cd E:\STEM_VCS\previous_Versions\version_0
echo    stem push
echo.
echo ============================================================================
echo.
echo Press any key to close this window...
pause >nul

REM Refresh environment variables
echo Refreshing system...
taskkill /f /im explorer.exe >nul 2>&1
start explorer.exe
