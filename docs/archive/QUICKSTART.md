# STEM VCS - Quick Start Guide

## ⚠️ Prerequisites Not Found

Your system doesn't have the required build tools installed. Here's how to get started:

## Option 1: Install Build Tools (Recommended)

### For Windows

#### Install Visual Studio Build Tools

1. Download **Visual Studio 2022 Community** (free):
   https://visualstudio.microsoft.com/downloads/

2. During installation, select:
   - ✅ **Desktop development with C++**
   - ✅ **CMake tools for Windows**

3. After installation, open **Developer Command Prompt for VS 2022**

4. Verify installation:
   ```cmd
   cl
   cmake --version
   ```

#### Install vcpkg (Package Manager)

```cmd
git clone https://github.com/Microsoft/vcpkg.git C:\vcpkg
cd C:\vcpkg
bootstrap-vcpkg.bat
vcpkg integrate install
```

#### Install Dependencies

```cmd
vcpkg install openssl:x64-windows zlib:x64-windows
```

## Option 2: Use MSYS2/MinGW (Alternative)

1. Download MSYS2: https://www.msys2.org/

2. Install and run MSYS2 MINGW64

3. Install tools:
   ```bash
   pacman -S mingw-w64-x86_64-gcc mingw-w64-x86_64-cmake make
   pacman -S mingw-w64-x86_64-openssl mingw-w64-x86_64-zlib
   ```

## Building STEM

### Step 1: Create Build Directory

```cmd
cd e:\STEM_VCS
mkdir build
cd build
```

### Step 2: Configure (Visual Studio)

```cmd
cmake .. -DBUILD_CLI=ON
```

### Step 3: Build

```cmd
cmake --build . --config Release
```

### Step 4: Run CLI

```cmd
src\cli\Release\stem.exe --version
src\cli\Release\stem.exe init
src\cli\Release\stem.exe status
```

## Building Without Dependencies (Core Only)

If you just want to test the core library without encryption:

```cmd
mkdir build-minimal
cd build-minimal
cmake .. -DBUILD_CLI=ON -DENABLE_ENCRYPTION=OFF
cmake --build . --config Release
```

## For GUI (Requires SDL2 + LVGL)

```cmd
# Install SDL2 first
vcpkg install sdl2:x64-windows

# Setup LVGL
cd e:\STEM_VCS
setup_lvgl.bat

# Build with GUI
mkdir build
cd build
cmake .. -DBUILD_GUI=ON -DBUILD_CLI=ON
cmake --build . --config Release

# Run GUI
src\gui\Release\stem_gui.exe
```

## Test Without Building

You can test the commands conceptually:

```cmd
# This shows what STEM will do
stem init
stem remote add cloud gdrive://MyDrive/Projects/MyRepo
stem add .
stem commit -m "Initial commit"
stem push
```

## Next Steps

1. **Install Visual Studio** or **MSYS2**
2. **Run build commands** above
3. **Try STEM commands**
4. **Read** [BUILD.md](BUILD.md) for details
5. **Contribute** - see [CONTRIBUTING.md](CONTRIBUTING.md)

## Common Issues

### "cmake not found"
- Install CMake from https://cmake.org/download/
- Or install Visual Studio with CMake tools

### "Cannot open source file"
- Install OpenSSL: `vcpkg install openssl`
- Or disable encryption: `-DENABLE_ENCRYPTION=OFF`

### "SDL2 not found"  
- Install SDL2: `vcpkg install sdl2`
- Or build CLI only: `-DBUILD_GUI=OFF`

## Getting Help

- 📖 Read [BUILD.md](BUILD.md) for detailed instructions
- 🐛 Report issues: https://github.com/yourusername/stem-vcs/issues
- 💬 Ask questions in Discussions

---

**Don't have time to set up build tools?**

We'll provide pre-built binaries soon! Star the repo to get notified.
