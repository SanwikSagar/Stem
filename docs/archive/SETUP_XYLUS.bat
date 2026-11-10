@echo off
cls
echo ============================================================================
echo           Setting up .stemignore for Xylus OS Project
echo ============================================================================
echo.
echo This will create a .stemignore file in your Xylus OS project.
echo.
echo Files that will be EXCLUDED from commits:
echo   - .vscode/ folder
echo   - .kiro/ folder  
echo   - .vs/ folder
echo   - bin/ and obj/ folders
echo   - *.exe, *.dll, *.obj files
echo   - *.log, *.tmp files
echo   - IDE configuration files
echo.
echo Files that will be INCLUDED:
echo   - All source code (.c, .cpp, .h, .cs, etc.)
echo   - Assets (images, sounds, etc.)
echo   - Configuration files
echo   - Documentation
echo   - Build scripts
echo.
pause

echo.
echo Creating .stemignore file...
echo.

set "PROJECT_DIR=E:\STEM_VCS\previous_Versions\version_0"

(
echo # STEM Ignore File for Xylus OS
echo # Files and folders listed here will NOT be committed
echo.
echo # IDE Files
echo .vscode/
echo .vs/
echo .idea/
echo *.suo
echo *.user
echo.
echo # AI Assistant
echo .kiro/
echo.
echo # Build Outputs
echo bin/
echo obj/
echo build/
echo out/
echo *.exe
echo *.dll
echo *.so
echo *.o
echo *.obj
echo *.pdb
echo.
echo # Temporary Files
echo *.tmp
echo *.temp
echo *.log
echo *.cache
echo.
echo # OS Files
echo .DS_Store
echo Thumbs.db
echo desktop.ini
echo.
echo # Dependencies
echo node_modules/
echo packages/
echo.
echo # Secrets
echo *.key
echo .env
echo secrets.json
) > "%PROJECT_DIR%\.stemignore"

if exist "%PROJECT_DIR%\.stemignore" (
    echo [OK] Created .stemignore in Xylus OS project
    echo.
    echo Location: %PROJECT_DIR%\.stemignore
    echo.
    echo.
    echo ============================================================================
    echo                            Next Steps
    echo ============================================================================
    echo.
    echo 1. Review the .stemignore file (optional)
    echo 2. Stage all files:
    echo       cd %PROJECT_DIR%
    echo       stem add .
    echo.
    echo 3. Check what will be committed:
    echo       stem status
    echo.
    echo 4. Create commit:
    echo       stem commit -m "Initial commit: Xylus OS version 0"
    echo.
    echo 5. Push to Google Drive:
    echo       stem push
    echo.
    echo ============================================================================
) else (
    echo [ERROR] Failed to create .stemignore
    echo.
    echo Manual creation:
    echo 1. Navigate to: %PROJECT_DIR%
    echo 2. Create file: .stemignore
    echo 3. Copy contents from: .stemignore.template
)

echo.
pause
