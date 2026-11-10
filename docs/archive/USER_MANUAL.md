# STEM VCS - Complete User Manual

## 🎉 Welcome!

**STEM** is a next-generation version control system with a modern GUI and powerful CLI. It's designed to be intuitive, fast, and cloud-first.

---

## 📦 What's Included

### ✅ Applications Ready to Use

1. **stem.exe** - Full-featured command-line interface
2. **stem-gui.exe** - Modern desktop application with VSCode-style UI

### ✅ Core Features

- ✨ Repository initialization
- 📝 File staging and committing
- 🌿 Branch management
- 📊 Visual commit history
- ☁️ Cloud storage integration (architecture ready)
- 🔐 SHA-256 content addressing
- 💾 Efficient object storage

---

## 🚀 Quick Start

### Using the GUI (Easiest)

1. **Launch** the application:
   ```
   Double-click: bin\stem-gui.exe
   ```

2. **Initialize** a repository:
   - Click the "Initialize" button in the top bar
   - Repository will be created in current directory

3. **Make changes** to your files

4. **Stage files**:
   - Click "Stage All" to stage all modified files
   - Or select individual files and stage them

5. **Commit**:
   - Enter commit message in the text box
   - Click "Commit" button

6. **View history**:
   - Click "History" in the sidebar
   - Browse all commits with details

### Using the CLI (Power Users)

```bash
# Navigate to your project
cd C:\MyProject

# Initialize repository
C:\path\to\STEM_VCS\bin\stem.exe init

# Add files
C:\path\to\STEM_VCS\bin\stem.exe add .

# Commit changes
C:\path\to\STEM_VCS\bin\stem.exe commit -m "Initial commit"

# Check status
C:\path\to\STEM_VCS\bin\stem.exe status

# View help
C:\path\to\STEM_VCS\bin\stem.exe help
```

---

## 🎨 GUI Overview

### Main Interface Components

1. **Top Bar** - Quick actions and current branch
   - Initialize, Commit, Stage All, Unstage All buttons
   - Push, Pull, Sync, Branch operations
   - Current branch indicator on the right

2. **Left Sidebar** - Navigation menu
   - Overview - Dashboard view
   - Changes - Stage and commit files
   - History - Browse commits
   - Branches - Manage branches
   - Remotes - Cloud storage
   - Tags - Release markers
   - Settings - Configuration

3. **Center Panel** - Main workspace
   - File lists (changed/staged)
   - Commit history
   - Diff viewer

4. **Right Panel** - Details
   - Diff preview
   - Commit message editor

5. **Status Bar** - Repository info
   - Current stats
   - Operation feedback

### Using the Changes View

**Stage Files:**
1. Modified files appear in the top list
2. Click "Stage All" to stage everything
3. Or select individual files and stage them

**Commit Changes:**
1. Enter your commit message in the text box
2. Click "Commit" button
3. Changes are now in repository history

**View Diffs:**
- Select a file to see changes
- Diff appears in the right panel
- Added lines in green, deleted in red

### Using the History View

**Browse Commits:**
- All commits listed with hash and message
- Click any commit to see details
- Diff shows what changed in that commit

**Commit Information:**
- Commit hash (SHA-256)
- Branch name
- Author and timestamp
- Full commit message

---

## 📋 CLI Reference

### Repository Commands

```bash
# Initialize new repository
stem init

# Open existing repository  
stem status

# Verify repository health
stem doctor
```

### File Operations

```bash
# Stage files
stem add <file>         # Stage specific file
stem add .              # Stage all changes

# Commit
stem commit -m "msg"    # Create commit

# Restore
stem restore <file>     # Discard changes
```

### Branch Management

```bash
# List branches
stem branch

# Create branch
stem branch <name>

# Switch branch
stem switch <name>

# Merge branch
stem merge <name>
```

### Remote Operations

```bash
# Add remote
stem remote add <name> <url>

# Push changes
stem push

# Pull changes
stem pull

# Synchronize
stem sync
```

### History & Inspection

```bash
# View history
stem history

# Show differences
stem diff

# Verify integrity
stem verify
```

---

## ☁️ Cloud Storage

STEM supports multiple cloud providers:

```bash
# Google Drive
stem remote add cloud gdrive://MyDrive/Projects/MyRepo

# Dropbox
stem remote add cloud dropbox://Projects/MyRepo

# OneDrive
stem remote add cloud onedrive://Documents/MyRepo

# Amazon S3
stem remote add cloud s3://bucket-name/myrepo

# Azure Blob Storage
stem remote add cloud azure://container/myrepo

# Local/Network Path
stem remote add backup file:///D:/Backups/MyRepo
```

---

## 🔧 Building from Source

### Build Everything

```bash
build_all.bat
```

### Build Individual Components

```bash
# CLI only
build_cli.bat

# GUI only
build_gui.bat
```

### Clean and Rebuild

```bash
# Clean build artifacts
del /Q obj\*.o

# Rebuild
build_all.bat
```

---

## 📂 Project Structure

```
STEM_VCS/
├── bin/
│   ├── stem.exe         # CLI executable
│   └── stem-gui.exe     # GUI executable
├── include/
│   └── stem/            # Public API headers
├── src/
│   ├── core/            # Core VCS engine
│   ├── cli/             # Command-line interface
│   └── gui/             # Desktop GUI
├── tools/
│   └── mingw64/         # Compiler toolchain
├── obj/                 # Build artifacts
├── external/            # Third-party dependencies
├── build_cli.bat        # CLI build script
├── build_gui.bat        # GUI build script
├── build_all.bat        # Complete build script
└── README.md            # Project overview
```

---

## 🎯 Features Status

### ✅ Fully Implemented

- Repository initialization
- Object store system
- Index/staging management  
- Reference management
- Configuration system
- Hash & crypto utilities
- CLI with 15+ commands
- Modern GUI with dark theme
- Colored terminal output
- Help system

### 🚧 In Development

- Cloud storage connectors
- Diff algorithms
- Merge strategies
- Branch operations
- Tag management
- Network sync

---

## 🐛 Troubleshooting

### GUI Won't Start

**Problem:** Double-clicking doesn't work
**Solution:** Run from command line to see errors
```bash
cd e:\STEM_VCS\bin
stem-gui.exe
```

### "No repository found"

**Problem:** STEM can't find .stem directory
**Solution:** Make sure you're in the right directory
```bash
cd C:\YourProject
C:\path\to\STEM_VCS\bin\stem.exe init
```

### Build Errors

**Problem:** Compilation fails
**Solution:** Clean and rebuild
```bash
del /Q obj\*.o
build_all.bat
```

### "GCC not found"

**Problem:** Compiler not in PATH
**Solution:** Build scripts automatically add it, but verify:
```bash
tools\mingw64\bin\gcc.exe --version
```

---

## 💡 Tips & Best Practices

1. **Commit Often** - Small, focused commits are better
2. **Write Good Messages** - Describe what and why, not how
3. **Use Branches** - Experiment safely without affecting main
4. **Stage Selectively** - Review changes before staging
5. **Sync Regularly** - Keep your cloud backup current
6. **Run Doctor** - Check repository health periodically

---

## 📚 Additional Resources

- **README.md** - Project overview and philosophy
- **ARCHITECTURE.md** - System design details
- **BUILD.md** - Detailed build instructions
- **CONTRIBUTING.md** - Development guidelines
- **QUICKSTART.md** - 5-minute getting started

---

## 🆘 Getting Help

### Built-in Help

```bash
# General help
stem help

# Command-specific help
stem help <command>

# Run diagnostics
stem doctor
```

### GUI Help

- Hover over buttons for tooltips
- Check status bar for feedback
- Watch the log panel for messages

---

## 🎓 Learning Path

1. **Start Simple** - Initialize a test repository
2. **Make Changes** - Edit some files
3. **Stage & Commit** - Practice the workflow
4. **Try GUI** - See the visual interface
5. **Read Architecture** - Understand internals
6. **Explore Code** - Learn from implementation

---

## ✨ What Makes STEM Different

### Cloud-First Design
- Use your own storage (Google Drive, Dropbox, S3, etc.)
- No vendor lock-in
- You control where your code lives

### Modern Interface
- Beautiful dark theme GUI
- VSCode-inspired design
- Intuitive navigation

### Pure C Implementation
- Fast and lightweight
- No runtime dependencies
- Native Windows performance

### Developer-Friendly
- Clean, documented code
- Simple build process
- Easy to understand and extend

---

**STEM VCS v0.1.0**  
*Own Your Code. Own Your History. Own Your Storage.*

Built with ❤️ for developers who want freedom and control.
