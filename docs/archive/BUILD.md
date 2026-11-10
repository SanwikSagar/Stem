# Building STEM from Source

This guide covers building STEM on Windows, macOS, and Linux.

## Prerequisites

### All Platforms

- **CMake** 3.20 or higher
- **C99 compiler** (GCC, Clang, or MSVC)
- **Git** for cloning the repository

### Dependencies

- **OpenSSL** (for cryptography)
- **zlib** (for compression)
- **SDL2** (for GUI, optional)
- **LVGL** (included as submodule)

## Platform-Specific Setup

### Windows

#### Using vcpkg (Recommended)

```cmd
REM Install vcpkg
git clone https://github.com/Microsoft/vcpkg.git
cd vcpkg
bootstrap-vcpkg.bat

REM Install dependencies
vcpkg install sdl2 openssl zlib

REM Note the vcpkg root path for later
set VCPKG_ROOT=%CD%
```

#### Build

```cmd
git clone https://github.com/yourusername/stem-vcs
cd stem-vcs

REM Setup LVGL
setup_lvgl.bat

REM Create build directory
mkdir build
cd build

REM Configure with vcpkg
cmake .. -DCMAKE_TOOLCHAIN_FILE=%VCPKG_ROOT%\scripts\buildsystems\vcpkg.cmake ^
         -DBUILD_GUI=ON ^
         -DBUILD_CLI=ON

REM Build
cmake --build . --config Release

REM Run
src\cli\Release\stem.exe --version
src\gui\Release\stem_gui.exe
```

### macOS

#### Using Homebrew

```bash
# Install Homebrew if not already installed
/bin/bash -c "$(curl -fsSL https://raw.githubusercontent.com/Homebrew/install/HEAD/install.sh)"

# Install dependencies
brew install cmake sdl2 openssl zlib

# Clone and build
git clone https://github.com/yourusername/stem-vcs
cd stem-vcs

# Setup LVGL
./setup_lvgl.sh

# Build
mkdir build && cd build
cmake .. -DBUILD_GUI=ON -DBUILD_CLI=ON
cmake --build . --config Release

# Run
./src/cli/stem --version
./src/gui/stem_gui
```

### Linux (Ubuntu/Debian)

```bash
# Install dependencies
sudo apt update
sudo apt install build-essential cmake git libsdl2-dev libssl-dev zlib1g-dev

# Clone and build
git clone https://github.com/yourusername/stem-vcs
cd stem-vcs

# Setup LVGL
chmod +x setup_lvgl.sh
./setup_lvgl.sh

# Build
mkdir build && cd build
cmake .. -DBUILD_GUI=ON -DBUILD_CLI=ON
cmake --build . --config Release

# Install (optional)
sudo cmake --install .

# Run
stem --version
stem_gui
```

### Linux (Fedora/RHEL)

```bash
# Install dependencies
sudo dnf install cmake gcc SDL2-devel openssl-devel zlib-devel

# Clone and build (same as Ubuntu)
git clone https://github.com/yourusername/stem-vcs
cd stem-vcs
./setup_lvgl.sh
mkdir build && cd build
cmake .. -DBUILD_GUI=ON -DBUILD_CLI=ON
cmake --build . --config Release
```

## Build Options

Configure with CMake options:

```bash
cmake .. \
  -DBUILD_SHARED_LIBS=ON    # Build shared libraries (default: ON)
  -DBUILD_CLI=ON             # Build CLI tool (default: ON)
  -DBUILD_GUI=ON             # Build GUI application (default: ON)
  -DBUILD_TESTS=ON           # Build tests (default: OFF)
  -DENABLE_ENCRYPTION=ON     # Enable encryption (default: ON)
  -DCMAKE_BUILD_TYPE=Release # Build type (Debug/Release)
```

## Testing

```bash
# Build with tests
cmake .. -DBUILD_TESTS=ON
cmake --build .

# Run tests
ctest --output-on-failure
```

## Installation

```bash
# Install to system
sudo cmake --install build

# Or install to custom prefix
cmake --install build --prefix /opt/stem

# Uninstall
sudo xargs rm < build/install_manifest.txt
```

## Troubleshooting

### OpenSSL not found

```bash
# macOS: Tell CMake where OpenSSL is
cmake .. -DOPENSSL_ROOT_DIR=/usr/local/opt/openssl

# Linux: Install development package
sudo apt install libssl-dev
```

### SDL2 not found

```bash
# Windows vcpkg
vcpkg install sdl2

# macOS
brew install sdl2

# Linux
sudo apt install libsdl2-dev
```

### LVGL not found

```bash
# Run the setup script
./setup_lvgl.sh  # Linux/macOS
setup_lvgl.bat    # Windows

# Or manually clone
git submodule update --init --recursive
```

### Build fails on Windows

- Ensure you're using Visual Studio 2019 or newer
- Run from "Developer Command Prompt for VS"
- Check vcpkg integration: `vcpkg integrate install`

### Permission denied (Linux/macOS)

```bash
# Make scripts executable
chmod +x setup_lvgl.sh

# Use sudo for system install
sudo cmake --install build
```

## Development Build

For faster iteration during development:

```bash
mkdir build-dev && cd build-dev
cmake .. -DCMAKE_BUILD_TYPE=Debug -DBUILD_TESTS=ON
cmake --build .

# Use ccache for faster rebuilds
cmake .. -DCMAKE_C_COMPILER_LAUNCHER=ccache
```

## Cross-Compilation

### Windows → Linux (WSL)

```bash
# From WSL
cmake .. -DCMAKE_TOOLCHAIN_FILE=../cmake/linux.cmake
cmake --build .
```

### macOS → iOS (Future)

```bash
cmake .. -DCMAKE_TOOLCHAIN_FILE=../cmake/ios.cmake
cmake --build .
```

## Binary Size Optimization

```bash
# Strip symbols
cmake .. -DCMAKE_BUILD_TYPE=MinSizeRel
cmake --build .
strip build/src/cli/stem
strip build/src/gui/stem_gui

# Use LTO
cmake .. -DCMAKE_INTERPROCEDURAL_OPTIMIZATION=ON
```

## Next Steps

- Read [CONTRIBUTING.md](CONTRIBUTING.md) for development guidelines
- Check [README.md](README.md) for usage instructions
- Report build issues at https://github.com/yourusername/stem-vcs/issues
