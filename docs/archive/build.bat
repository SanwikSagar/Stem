@echo off
setlocal enabledelayedexpansion

echo ========================================
echo   STEM VCS - Windows Build Script
echo ========================================
echo.

REM Check for Visual Studio
where cl >nul 2>&1
if %ERRORLEVEL% NEQ 0 (
    echo [ERROR] Visual Studio compiler not found!
    echo.
    echo Please run this script from:
    echo   "Developer Command Prompt for VS 2022"
    echo.
    echo Or install Visual Studio 2022 with C++ tools from:
    echo   https://visualstudio.microsoft.com/downloads/
    echo.
    pause
    exit /b 1
)

REM Check for CMake
where cmake >nul 2>&1
if %ERRORLEVEL% NEQ 0 (
    echo [ERROR] CMake not found!
    echo.
    echo Install CMake from: https://cmake.org/download/
    echo.
    pause
    exit /b 1
)

echo [OK] Build tools found
echo.

REM Create build directory
if not exist build mkdir build
cd build

echo [INFO] Configuring STEM VCS...
echo.

REM Configure with CMake
cmake .. -G "Visual Studio 17 2022" -A x64 ^
    -DBUILD_CLI=ON ^
    -DBUILD_GUI=OFF ^
    -DENABLE_ENCRYPTION=OFF

if %ERRORLEVEL% NEQ 0 (
    echo.
    echo [ERROR] CMake configuration failed!
    cd ..
    pause
    exit /b 1
)

echo.
echo [INFO] Building STEM VCS...
echo.

REM Build Release configuration
cmake --build . --config Release

if %ERRORLEVEL% NEQ 0 (
    echo.
    echo [ERROR] Build failed!
    cd ..
    pause
    exit /b 1
)

echo.
echo ========================================
echo   BUILD SUCCESSFUL!
echo ========================================
echo.
echo CLI executable: build\src\cli\Release\stem.exe
echo.
echo Test it:
echo   cd build\src\cli\Release
echo   stem.exe --version
echo   stem.exe help
echo.

cd ..
pause
