@echo off
REM Add STEM to PATH for the current CMD session
set PATH=%PATH%;%~dp0bin

cls
echo.
echo ============================================================================
echo                  STEM Added to Current Session
echo ============================================================================
echo.
stem --version
echo.
echo ============================================================================
echo.
echo You can now use 'stem' in this window!
echo.
echo Examples:
echo   stem help
echo   stem status  
echo   stem push
echo.
echo For your Xylus OS project:
echo   cd E:\STEM_VCS\previous_Versions\version_0
echo   stem push
echo.
echo ============================================================================
echo.

