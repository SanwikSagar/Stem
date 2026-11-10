@echo off
setlocal EnableDelayedExpansion

:: STEM VCS - Windows Installation
:: Simple, fast, cross-platform VCS

cls
echo.
echo ================================================================
echo          STEM VCS - Installation Wizard
echo ================================================================
echo.

:: Check GCC
echo [*] Checking for GCC...
set "MINGW_PATH=%~dp0tools\mingw64\bin"
if exist "%MINGW_PATH%\gcc.exe" (
    echo [OK] Found bundled MinGW GCC
    set "PATH=%MINGW_PATH%;%PATH%"
) else (
    where gcc >nul 2>&1
    if errorlevel 1 (
        if not exist "%~dp0tools\mingw.7z" (
            echo [ERROR] GCC not found and tools\mingw.7z is missing.
            pause
            exit /b 1
        )
        for %%A in ("%~dp0tools\mingw.7z") do set "MINGW_ARCHIVE_SIZE=%%~zA"
        if !MINGW_ARCHIVE_SIZE! LSS 1048576 (
            echo [ERROR] MinGW archive is a Git LFS pointer, not the downloaded bundle.
            echo Install Git LFS, then run: git lfs install ^&^& git lfs pull
            pause
            exit /b 1
        )
        echo [ERROR] GCC not found. Extract tools\mingw.7z to tools\mingw64 first.
        pause
        exit /b 1
    )
    echo [OK] Found system GCC
)

echo [*] Checking PowerShell...
where powershell >nul 2>&1
if errorlevel 1 (
    echo [ERROR] PowerShell not found!
    pause
    exit /b 1
)
echo [OK] PowerShell found
echo.

:: Build
echo ================================================================
echo   Building STEM VCS
echo ================================================================
echo.
call build_cli.bat
if errorlevel 1 (
    echo [ERROR] Build failed!
    pause
    exit /b 1
)

:: Test
echo ================================================================
echo   Testing
echo ================================================================
echo.
bin\stem.exe --version
if errorlevel 1 (
    echo [ERROR] Build test failed!
    pause
    exit /b 1
)
echo.
echo [OK] Build successful!
echo.

:: PATH
echo ================================================================
echo   Adding to PATH
echo ================================================================
echo.
set "STEM_BIN=%~dp0bin"
echo Location: %STEM_BIN%
echo.
choice /C YN /M "Add to PATH"
if errorlevel 2 (
    echo [SKIPPED] Run manually: %STEM_BIN%\stem.exe
) else (
    powershell -Command "[Environment]::SetEnvironmentVariable('Path', [Environment]::GetEnvironmentVariable('Path', 'User') + ';%STEM_BIN%', 'User')" 2>nul
    if errorlevel 1 (
        echo [WARNING] Manual setup needed - see MANUAL.md
    ) else (
        echo [OK] Added to PATH - restart terminal!
    )
)
echo.

:: Google Drive
echo ================================================================
echo   Google Drive Setup (Optional)
echo ================================================================
echo.
choice /C YN /M "Setup Google Drive"
if errorlevel 2 (
    echo [SKIPPED] Setup later: gdrive_upload.ps1 -SetupAuth
) else (
    powershell -ExecutionPolicy Bypass -File gdrive_upload.ps1 -SetupAuth
)

:: Done
echo.
echo ================================================================
echo   Installation Complete!
echo ================================================================
echo.
echo [OK] STEM VCS is ready!
echo.
echo Next Steps:
echo   1. Restart terminal
echo   2. stem init
echo   3. stem add .
echo   4. stem commit -m "message"
echo   5. stem push
echo.
echo Docs: README.md, MANUAL.md
echo Help: stem --help
echo.
pause
