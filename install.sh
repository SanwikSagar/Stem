#!/bin/bash
# STEM VCS - Universal Installation Script
# Supports: Linux, macOS, Windows (Git Bash/MSYS2)

set -e

# Colors
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m' # No Color

# Detect OS
detect_os() {
    if [[ "$OSTYPE" == "linux-gnu"* ]]; then
        echo "linux"
    elif [[ "$OSTYPE" == "darwin"* ]]; then
        echo "macos"
    elif [[ "$OSTYPE" == "msys" ]] || [[ "$OSTYPE" == "win32" ]]; then
        echo "windows"
    else
        echo "unknown"
    fi
}

OS=$(detect_os)

echo ""
echo "================================================================"
echo "          STEM VCS - Installation Wizard"
echo "================================================================"
echo ""
echo "Platform detected: $OS"
echo ""

# Check for compiler
echo "[*] Checking for GCC..."
if command -v gcc &> /dev/null; then
    GCC_VERSION=$(gcc --version | head -n1)
    echo -e "${GREEN}[OK]${NC} Found: $GCC_VERSION"
else
    echo -e "${RED}[ERROR]${NC} GCC not found!"
    echo ""
    echo "Please install GCC:"
    if [ "$OS" == "linux" ]; then
        echo "  Ubuntu/Debian: sudo apt install build-essential"
        echo "  Fedora/RHEL:   sudo dnf install gcc make"
        echo "  Arch:          sudo pacman -S base-devel"
    elif [ "$OS" == "macos" ]; then
        echo "  Install Xcode Command Line Tools:"
        echo "  xcode-select --install"
    elif [ "$OS" == "windows" ]; then
        echo "  Install MinGW-w64 or use build_cli.bat"
    fi
    exit 1
fi

# Check for make
echo "[*] Checking for Make..."
if command -v make &> /dev/null; then
    echo -e "${GREEN}[OK]${NC} Make found"
else
    echo -e "${YELLOW}[WARNING]${NC} Make not found, will use fallback"
fi

echo ""
echo "================================================================"
echo "          Building STEM VCS"
echo "================================================================"
echo ""

# Build
if command -v make &> /dev/null; then
    make clean 2>/dev/null || true
    make
else
    echo "[*] Using direct compilation..."
    mkdir -p obj bin
    
    # Compile core
    for file in src/core/*.c; do
        echo "[CC] $file"
        gcc -O2 -Wall -Wextra -Iinclude -std=c11 -c "$file" -o "obj/$(basename ${file%.c}.o)"
    done
    
    # Compile CLI
    for file in src/cli/*.c; do
        echo "[CC] $file"
        gcc -O2 -Wall -Wextra -Iinclude -std=c11 -c "$file" -o "obj/$(basename ${file%.c}.o)"
    done
    
    # Link
    echo ""
    echo "[LINK] Creating executable..."
    if [ "$OS" == "windows" ]; then
        gcc obj/*.o -o bin/stem.exe -lws2_32
    else
        gcc obj/*.o -o bin/stem
    fi
    
    echo ""
    echo -e "${GREEN}BUILD SUCCESSFUL!${NC}"
fi

# Test
echo ""
echo "[*] Testing build..."
if [ "$OS" == "windows" ]; then
    ./bin/stem.exe --version
else
    ./bin/stem --version
fi

echo ""
echo "================================================================"
echo "          Installation"
echo "================================================================"
echo ""

# Install based on OS
if [ "$OS" == "linux" ] || [ "$OS" == "macos" ]; then
    echo "Install STEM VCS to /usr/local/bin? (requires sudo)"
    read -p "Install? (y/n) " -n 1 -r
    echo
    if [[ $REPLY =~ ^[Yy]$ ]]; then
        sudo make install 2>/dev/null || {
            sudo install -d /usr/local/bin
            sudo install -m 755 bin/stem /usr/local/bin/stem
        }
        echo ""
        echo -e "${GREEN}[OK]${NC} Installed to /usr/local/bin/stem"
        echo ""
        echo "Verify with: stem --version"
    else
        echo ""
        echo "STEM VCS built but not installed."
        echo "Run directly: ./bin/stem"
        echo ""
        echo "To install later:"
        echo "  sudo make install"
        echo "or:"
        echo "  sudo cp bin/stem /usr/local/bin/"
    fi
elif [ "$OS" == "windows" ]; then
    echo "On Windows, STEM is ready to use from:"
    echo "  $(pwd)/bin/stem.exe"
    echo ""
    echo "To add to PATH, run: INSTALL.bat"
fi

echo ""
echo "================================================================"
echo "          Setup Complete!"
echo "================================================================"
echo ""
echo "Next steps:"
echo "  1. stem init              - Initialize a repository"
echo "  2. stem config user.name \"Your Name\""
echo "  3. stem add .             - Stage files"
echo "  4. stem commit -m \"msg\"   - Create commit"
echo "  5. stem push              - Backup to cloud"
echo ""
echo "Documentation:"
echo "  - README.md        - Overview"
echo "  - MANUAL.md        - Complete guide"
echo "  - stem help        - Command reference"
echo ""
echo -e "${GREEN}Installation complete!${NC}"
echo ""
