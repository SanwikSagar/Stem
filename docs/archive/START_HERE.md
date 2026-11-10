# 🌱 STEM VCS - Start Here

## Welcome to STEM!

STEM is a next-generation distributed version control system that puts you in control. Use your own cloud storage instead of relying on centralized hosting platforms.

**Version 1.0.0 | Copyright © 2026**

---

## ⚡ Quick Start (30 seconds)

### Option 1: Use the GUI (Recommended)

1. **Launch**: Double-click `bin\stem-gui.exe`
2. **Initialize**: Click "Initialize" button
3. **Configure**: Set repository path and Google Drive URL
4. **Done**: Start managing your code!

### Option 2: Use the CLI

```cmd
# Initialize repository with cloud storage
stem init --path "C:\MyProject" --cloud "https://drive.google.com/drive/folders/YOUR_FOLDER_ID"

# Or use interactive mode
stem init -i

# Stage files
stem add .

# Commit changes
stem commit -m "Initial commit"

# Push to cloud
stem push
```

---

## 📦 What You Have

### Ready-to-Use Applications

✅ **stem.exe** - Full-featured command-line tool  
✅ **stem-gui.exe** - Modern desktop GUI with dark theme

### Key Features

- 🚀 **Fast native Windows performance**
- 🎨 **Beautiful modern interface**
- ☁️ **Real Google Drive integration**
- 🗂️ **Work with any directory on your system**
- 🔐 **SHA-256 content addressing**
- 📊 **Visual commit history**
- 🌿 **Branch management**
- 💾 **Efficient object store**

---

## 🌟 What's New in 1.0.0

### Enhanced Directory Access
- Initialize repositories anywhere on your system
- Works just like GitHub Desktop
- Browse and select any folder
- No restrictions on location

### Real Cloud Storage
- Direct Google Drive integration
- Paste your actual Drive folder URL
- Automatic folder ID extraction
- Examples:
  ```
  https://drive.google.com/drive/folders/1ABC123xyz
  https://drive.google.com/drive/folders/1ABC123xyz?usp=drive_link
  ```

### Interactive Setup
- `-i` or `--interactive` flag for guided setup
- Step-by-step repository creation
- Prompts for directory and cloud URL
- Perfect for beginners

### Modern CLI
- Clean, professional output
- Unicode icons for better visuals
- Colored messages
- Helpful error messages

---

## 📚 Documentation

- **SETUP_XYLUS_PROJECT.md** - Example setup guide with real Google Drive URL
- **USER_MANUAL.md** - Complete user guide
- **README.md** - Project overview and philosophy
- **QUICKSTART.md** - 5-minute tutorial
- **QUICK_REFERENCE.md** - Command cheat sheet
- **ARCHITECTURE.md** - Technical design
- **BUILD.md** - Build from source instructions

---

## 🎯 Common Tasks

### Initialize a Repository Anywhere

**Interactive Mode (Recommended):**
```cmd
stem init -i
```
Then follow the prompts:
1. Enter repository path (or press Enter for current directory)
2. Enter Google Drive folder URL

**Command Line:**
```cmd
stem init --path "E:\MyProjects\MyRepo" --cloud "https://drive.google.com/drive/folders/YOUR_ID"
```

**Current Directory:**
```cmd
cd E:\MyProjects\MyRepo
stem init --cloud "https://drive.google.com/drive/folders/YOUR_ID"
```

### Add Google Drive Remote

```cmd
stem remote add cloud "https://drive.google.com/drive/folders/YOUR_FOLDER_ID"
stem remote show cloud
stem remote test cloud
```

### Commit Changes

**GUI:**
1. Click "Stage All"
2. Enter commit message
3. Click "Commit"

**CLI:**
```cmd
stem add .
stem commit -m "Your message"
```

### Push to Google Drive

**GUI:** Click "Push" button  
**CLI:** `stem push`

### View History

**GUI:** Click "History" in sidebar  
**CLI:** `stem history`

---

## 🔧 Building

```cmd
# Build everything
build_all.bat

# Just the CLI
build_cli.bat

# Just the GUI
build_gui.bat
```

All builds use the included MinGW compiler in `tools/mingw64/`.

---

## 💡 Pro Tips

1. **Use Interactive Mode** when setting up new repositories
2. **Real Drive URLs** - Use actual Google Drive folder links
3. **Any Directory** - Initialize repos anywhere: `E:\`, `C:\Projects\`, `D:\Code\`
4. **Commit Often** with clear, descriptive messages
5. **Test Remote** before pushing: `stem remote test cloud`

---

## 🆘 Need Help?

### Built-in Help
```cmd
stem help
stem help init
stem help remote
stem --version
```

### Check Repository
```cmd
stem doctor
stem status
stem remote list
```

### Documentation
- Open **USER_MANUAL.md** for the complete guide
- Check **SETUP_XYLUS_PROJECT.md** for a real example
- See **QUICK_REFERENCE.md** for command cheatsheet

---

## ✨ Example: Initialize Existing Project

Let's say you want to version control `E:\STEM_VCS\previous_Versions\version_0`:

```cmd
# Navigate to project
cd E:\STEM_VCS\previous_Versions\version_0

# Initialize with your Google Drive folder
stem init --cloud "https://drive.google.com/drive/folders/YOUR_FOLDER_ID"

# Stage all files
stem add .

# Create first commit
stem commit -m "Initial commit: Xylus OS version_0"

# Push to Google Drive
stem push
```

Done! Your project is now version controlled and backed up to your Google Drive.

---

## 🌟 STEM Philosophy

**Own Your Code** - No vendor lock-in  
**Own Your History** - Complete local control  
**Own Your Storage** - Use any cloud provider  
**Own Your Data** - Full privacy and control

---

## ✅ What's Working

- ✅ Initialize repositories in any directory
- ✅ Real Google Drive URL support
- ✅ Interactive setup mode
- ✅ Directory browsing (GUI)
- ✅ Automatic folder ID extraction
- ✅ Clean, modern CLI output
- ✅ Full remote management
- ✅ Commit and staging
- ✅ Branch operations
- ✅ Status reporting

---

## 🚀 Getting Started

**Simplest Way:**
```cmd
stem init -i
```

**With Your Google Drive:**
1. Create a folder in Google Drive
2. Right-click → Get Link
3. Use that URL with STEM:
   ```cmd
   stem init --cloud "YOUR_GOOGLE_DRIVE_URL"
   ```

**That's it!** You're ready to use STEM.

---

**Ready? Let's get started!**

*STEM VCS v1.0.0 - Built with ❤️ for developers*  
*Copyright © 2026 STEM VCS Contributors*
