@echo off
echo Cleaning STEM VCS project directory...
echo.

REM Remove temporary build artifacts
if exist obj\*.o del /Q obj\*.o 2>nul
if exist bin\*.exe echo Keeping executables in bin\

REM Remove test directory
if exist test_repo rmdir /S /Q test_repo 2>nul

REM Remove old documentation files
del /Q LVGL_MIGRATION.md MIGRATION_PLAN.md PROJECT_COMPLETE.md PROJECT_STATUS.md TASK_PROGRESS.md SUMMARY.md README_FIRST.txt INSTALL_COMPILER.md INSTALL_TOOLS.md 2>nul

REM Remove old build scripts
del /Q build_simple.bat build_portable.bat setup_and_build.bat setup_and_build.ps1 setup_lvgl.bat setup_lvgl.sh compile.bat test_stem.bat build_gui_win32.bat 2>nul

echo.
echo ========================================
echo   Project Cleaned Successfully!
echo ========================================
echo.
echo Remaining structure:
echo   ├── bin/           - Executable files
echo   ├── include/       - Header files
echo   ├── src/           - Source code
echo   ├── tools/         - Compiler (MinGW)
echo   └── Documentation  - Essential docs only
echo.
pause
