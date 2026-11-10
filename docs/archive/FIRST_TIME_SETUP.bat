@echo off
echo ================================================================================
echo                   STEM VCS - First Time Google Drive Setup
echo ================================================================================
echo.
echo This script will help you authorize STEM to upload to Google Drive.
echo.
echo What happens:
echo   1. Browser opens to Google authorization page
echo   2. You sign in and allow STEM VCS
echo   3. Google gives you a code
echo   4. You paste it here
echo   5. Done! Authorization saved for future use
echo.
echo Press any key to continue...
pause >nul

powershell -ExecutionPolicy Bypass -File "%~dp0gdrive_upload.ps1" -SetupAuth

echo.
echo ================================================================================
echo.
echo If authorization was successful, you can now use 'stem push' to upload files!
echo.
echo Next steps:
echo   cd "E:\STEM_VCS\previous_Versions\version_0"
echo   stem push
echo.
echo ================================================================================
pause
