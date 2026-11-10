@echo off
echo ============================================================================
echo                    Pushing Xylus OS to Google Drive
echo ============================================================================
echo.

set "STEM_EXE=%~dp0bin\stem.exe"

cd /d "E:\STEM_VCS\previous_Versions\version_0"

echo Current directory: %CD%
echo.

"%STEM_EXE%" status
echo.

echo Pushing to Google Drive...
"%STEM_EXE%" push

echo.
echo ============================================================================
echo Done!
echo ============================================================================
pause
