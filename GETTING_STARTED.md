# Getting Started with STEM VCS

**Quick guide to install and use STEM in 5 minutes**

---

## Step 1: Install STEM

### Windows
```cmd
INSTALL.bat
```
The installer includes MinGW GCC - no additional downloads needed!

### macOS
```bash
# First time: Install Xcode Command Line Tools (if not already installed)
xcode-select --install

# Then install STEM
chmod +x install.sh
./install.sh
```

### Linux
```bash
# First time: Install build tools (if not already installed)
# Ubuntu/Debian:
sudo apt install build-essential

# Fedora/RHEL:
sudo dnf install gcc make

# Arch:
sudo pacman -S base-devel

# Then install STEM
chmod +x install.sh
./install.sh
```

### What the Installer Does
1. ✅ Checks for prerequisites (GCC, Make, PowerShell on Windows)
2. ✅ Builds STEM from source
3. ✅ Tests the build
4. ✅ Optionally adds to PATH
5. ✅ Optionally sets up Google Drive authentication

**Then restart your terminal!**

---

## Step 2: Initialize Your Project

Navigate to your project and initialize:

```bash
cd /path/to/your/project
stem init
```

---

## Step 3: Add Files

Add all files (creates `.stemignore` on first use):

```bash
stem add .
```

Review and customize `.stemignore` to exclude build outputs, temp files, etc.

---

## Step 4: Commit

Create a commit:

```bash
stem commit -m "Initial commit"
```

---

## Step 5: Push to Google Drive (Optional)

Add your Google Drive folder:

Before the first push, create a Google OAuth desktop client and set its values in your PowerShell session:

```powershell
$env:STEM_GDRIVE_CLIENT_ID = "YOUR_CLIENT_ID.apps.googleusercontent.com"
$env:STEM_GDRIVE_CLIENT_SECRET = "YOUR_CLIENT_SECRET"
```

Keep the client secret private. These session variables are not saved after PowerShell closes.

```bash
stem remote add cloud https://drive.google.com/drive/folders/YOUR-FOLDER-ID
```

**How to get the folder ID:**
1. Create a folder in Google Drive
2. Open it and copy the URL
3. The ID is after `folders/`

Push your project:

```bash
stem push
```

First time: Browser opens for authorization → Click "Allow" → Done!

---

## Common Commands

```bash
stem init                          # Initialize repository
stem add .                         # Stage all files
stem add file.txt                  # Stage specific file
stem commit -m "message"           # Create commit
stem remote add cloud URL          # Add Google Drive (optional)
stem push                          # Upload to cloud
stem help                          # Show all commands
```

---

## Need More Help?

- **Complete Manual:** `MANUAL.md`
- **Quick Reference:** `README.md`
- **Feature List:** `FEATURES.md`
- **Troubleshooting:** See below

---

## Platform-Specific Troubleshooting

### Windows

**Problem:** "GCC not found" or the MinGW archive is a Git LFS pointer
- **Solution:** Install Git LFS, run `git lfs install` and `git lfs pull`, extract `tools/mingw.7z` to `tools/mingw64`, then run `INSTALL.bat`

**Problem:** PowerShell script won't run
- **Solution:** Run: `powershell -ExecutionPolicy Bypass -File gdrive_upload.ps1 -SetupAuth`

**Problem:** Not in PATH after install
- **Solution:** Manually add `C:\path\to\STEM_VCS\bin` to your PATH in System Environment Variables

### macOS

**Problem:** "Command not found: gcc"
- **Solution:** Install Xcode Command Line Tools: `xcode-select --install`

**Problem:** Permission denied during install
- **Solution:** Use sudo: `sudo ./install.sh` or `sudo make install`

**Problem:** "Unknown option: --version"
- **Solution:** You might have an old version. Rebuild: `make clean && make`

### Linux

**Problem:** "gcc: command not found"
- **Solution:** Install build tools:
  - Ubuntu/Debian: `sudo apt install build-essential`
  - Fedora: `sudo dnf install gcc make`
  - Arch: `sudo pacman -S base-devel`

**Problem:** Permission denied during install
- **Solution:** Use sudo for system-wide install: `sudo make install`

**Problem:** "stem: not found" after install
- **Solution:** Check if `/usr/local/bin` is in PATH: `echo $PATH`
  - Add if needed: `export PATH=/usr/local/bin:$PATH`
  - Make permanent: Add to `~/.bashrc` or `~/.zshrc`

### All Platforms

**Problem:** "Failed to initialize repository"
- **Solution:** Make sure you're in a valid directory with write permissions

**Problem:** Google Drive authentication fails
- **Solution:** 
  1. Delete existing token: Windows: `del %USERPROFILE%\.stem_gdrive_token`, Unix: `rm ~/.stem_gdrive_token`
  2. Run: `powershell gdrive_upload.ps1 -SetupAuth` (Windows) or setup from `stem push`
  3. Make sure port 8080 is not in use

**Problem:** "stem: command not found" after install
- **Solution:** Restart your terminal to refresh PATH, or run directly: `./bin/stem`

---

**That's it! You're ready to use STEM VCS.** 🚀
