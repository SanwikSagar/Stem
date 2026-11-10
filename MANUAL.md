# STEM VCS - Complete Manual

**Version 1.0.0** | **Copyright © 2026 STEM VCS Contributors**

---

## Table of Contents

1. [Introduction](#introduction)
2. [Installation](#installation)
3. [Quick Start Guide](#quick-start-guide)
4. [Command Reference](#command-reference)
5. [Google Drive Integration](#google-drive-integration)
6. [Working with .stemignore](#working-with-stemignore)
7. [Best Practices](#best-practices)
8. [Troubleshooting](#troubleshooting)
9. [FAQ](#faq)

---

## Introduction

### What is STEM VCS?

**STEM (Simple Team Efficient Management)** is a modern version control system designed for simplicity and cloud integration. It combines the essential features of version control with seamless Google Drive backup.

### Key Features

- ✅ **Simple Commands** - Easy-to-understand Git-like interface
- ✅ **Google Drive Integration** - Automatic cloud backup
- ✅ **Smart Filtering** - `.stemignore` excludes build artifacts and temp files
- ✅ **Directory Structure Preservation** - Maintains your project hierarchy
- ✅ **Windows Native** - Built with C for maximum performance
- ✅ **No Dependencies** - Works out of the box

### When to Use STEM

✅ **Good for:**
- Personal projects needing cloud backup
- Small team collaborations via Google Drive
- Projects where simplicity > advanced features
- Quick backups with version tracking

❌ **Not suitable for:**
- Large teams (use Git instead)
- Complex branching workflows
- Projects requiring advanced merge strategies

---

## Installation

### Prerequisites

- **Operating System:** Windows 10/11
- **Compiler:** MinGW-w64 GCC (included in package)
- **PowerShell:** For Google Drive integration
- **Google Account:** (Optional) For cloud backup

### Installation Steps

1. **Download** or clone the STEM VCS repository

2. **Run the installer:**
   ```cmd
   INSTALL.bat
   ```

3. **Follow the wizard:**
   - Step 1: Prerequisites check
   - Step 2: Build STEM binaries
   - Step 3: Add to system PATH
   - Step 4: Google Drive setup (optional)
   - Step 5: Verify installation

4. **Restart your terminal** to use `stem` command globally

### Verify Installation

```cmd
stem --version
```

Expected output:
```
STEM VCS version 1.0.0
Copyright (c) 2026 STEM VCS Contributors
```

---

## Quick Start Guide

### 1. Initialize a Repository

Navigate to your project folder and initialize:

```cmd
cd E:\MyProject
stem init
```

Output:
```
[OK] Initialized STEM repository in E:\MyProject\.stem
```

### 2. Create .stemignore File

When you first run `stem add .`, STEM will prompt you to create a `.stemignore` file:

```
Do you want to create a .stemignore file now? [Y/n]: y
```

This file tells STEM which files to ignore (build outputs, temp files, etc.).

### 3. Stage Your Files

Add all files to staging:

```cmd
stem add .
```

Or add specific files:

```cmd
stem add src/main.c
stem add README.md
```

### 4. Create a Commit

Create a snapshot of your staged files:

```cmd
stem commit -m "Initial commit"
```

### 5. Set Up Google Drive Remote

Add your Google Drive folder as a remote:

```cmd
stem remote add cloud https://drive.google.com/drive/folders/YOUR-FOLDER-ID
```

**How to get your folder ID:**
1. Create a folder in Google Drive
2. Open it and copy the URL
3. The ID is the last part: `folders/YOUR-FOLDER-ID`

### 6. Push to Google Drive

Upload your project to the cloud:

```cmd
stem push
```

The first time, you'll need to authorize STEM to access Google Drive.

---

## Command Reference

### Repository Management

#### `stem init`
Initialize a new STEM repository in the current directory.

```cmd
stem init
stem init --path E:\OtherProject
```

**Options:**
- `--path PATH` - Initialize in a different directory
- `-i` - Interactive mode with prompts

---

#### `stem remote`
Manage cloud storage remotes.

**Add a remote:**
```cmd
stem remote add cloud https://drive.google.com/drive/folders/FOLDER-ID
```

**List remotes:**
```cmd
stem remote list
```

**Remove a remote:**
```cmd
stem remote remove cloud
```

---

### File Operations

#### `stem add`
Stage files for commit.

```cmd
stem add .                    # Add all files
stem add src/                 # Add entire directory
stem add file.txt            # Add specific file
stem add *.c                 # Add files matching pattern
```

**What happens:**
- Files are added to `.stem/index`
- Respects `.stemignore` patterns
- Shows count of staged files

---

#### `stem commit`
Create a commit snapshot.

```cmd
stem commit -m "Your commit message"
```

**Options:**
- `-m MESSAGE` - Commit message (required)
- `--help` - Show help

**What happens:**
- Reads staged files from `.stem/index`
- Stores files in `.stem/objects/` with directory structure
- Creates commit ID and metadata
- Ready for push

---

#### `stem push`
Upload your project to Google Drive (compressed).

```cmd
stem push
stem push cloud              # Push to specific remote
```

**What happens:**
- Scans all project files (respecting `.stemignore`)
- Compresses into single .zip archive (typically 30-70% smaller)
- Uploads one file to Google Drive
- Shows compression ratio and speed
- **Much faster than uploading individual files!**

**Performance:**
- Small projects (< 100 MB): ~5-15 seconds total
- Large projects (> 500 MB): Minutes instead of hours
- Compression reduces upload time dramatically

**Note:** Archive format preserves complete directory structure.

---

#### `stem pull`
Download and sync from Google Drive.

```cmd
stem pull
stem pull cloud              # Pull from specific remote
```

**What happens:**
- Finds latest backup archive in Google Drive
- Downloads compressed archive
- Extracts all files to project directory
- Restores complete directory structure

**Note:** Overwrites local changes with cloud backup.

---

### Branching and Merging

#### `stem branch`
Manage branches in your repository.

**List branches:**
```cmd
stem branch                  # Show all branches
stem branch --list           # Alternative syntax
```

**Create a new branch:**
```cmd
stem branch feature-x        # Create branch named 'feature-x'
stem branch bugfix-123       # Create branch named 'bugfix-123'
```

**Delete a branch:**
```cmd
stem branch -d feature-x     # Delete branch 'feature-x'
stem branch --delete old-feature
```

**Options:**
- `(no args)` - List all branches
- `<name>` - Create new branch
- `-d, --delete <name>` - Delete branch
- `-l, --list` - List branches
- `--help` - Show help

**Branch naming rules:**
- No spaces or special characters (/, \, :, *, ?)
- Use hyphens or underscores: feature-x, bug_fix_1
- Case sensitive

---

#### `stem switch`
Switch to a different branch.

```cmd
stem switch main             # Switch to main branch
stem switch feature-x        # Switch to feature-x branch
stem switch -c new-feature   # Create and switch in one command
```

**Options:**
- `-c, --create` - Create branch if it doesn't exist
- `--help` - Show help

**What happens:**
- Updates `.stem/HEAD` to point to the new branch
- Future commits go to the new branch
- Preserves your working directory files

---

#### `stem merge`
Merge one branch into another.

```cmd
stem merge feature-x         # Merge feature-x into current branch
stem merge --abort           # Cancel ongoing merge
stem merge --continue        # Continue after resolving conflicts
```

**Merge strategies:**

1. **Fast-forward merge** (automatic)
   - Used when target branch hasn't diverged
   - Simply moves branch pointer forward
   - No merge commit needed

2. **3-way merge** (automatic)
   - Used when branches have diverged
   - Finds common ancestor
   - Merges changes from both branches
   - May require conflict resolution

**Conflict resolution workflow:**
```cmd
# Attempt merge
stem merge feature-x

# If conflicts occur:
# 1. Edit conflicting files manually
# 2. Stage resolved files
stem add conflicting-file.txt

# 3. Complete merge
stem commit -m "Merge feature-x"

# OR abort merge
stem merge --abort
```

---

### History and Inspection

#### `stem history`
View commit history.

```cmd
stem history                 # Show last 10 commits
stem history -n20            # Show last 20 commits
stem history --all           # Show all commits
```

**Output format:**
```
Commit: a1b2c3d
Date:   2026-08-03 14:30:45
Files:  25

    Initial commit

----------------------------------------------------------------

Commit: e4f5g6h
Date:   2026-08-02 10:15:22
Files:  18

    Added feature X
```

---

#### `stem diff`
Show changes between working directory and last commit.

```cmd
stem diff                    # Show all changes
stem diff file.txt           # Show changes in specific file
```

**Output format:**
```
--- file.txt (committed)
+++ file.txt (working)

- 1: old line
+ 1: new line
```

**Status indicators:**
- `-` = Line removed
- `+` = Line added
- (no indicator) = Line unchanged

---

#### `stem restore`
Restore files from last commit (discard local changes).

```cmd
stem restore file.txt        # Restore specific file
stem restore --all           # Restore all tracked files
```

**⚠️ WARNING:** This overwrites your local changes permanently!

---

#### `stem verify`
Check repository integrity.

```cmd
stem verify
```

**Checks:**
- `.stem/HEAD` file exists
- `.stem/refs/` directory structure
- `.stem/objects/` directory exists
- Reports any corruption or missing files

---

### Information Commands

#### `stem status`
Show repository status (planned feature).

```cmd
stem status
```

---

#### `stem help`
Show available commands.

```cmd
stem help
stem COMMAND --help          # Help for specific command
```

---

### Workflow Examples

#### Example 1: Feature Development
```cmd
# Create feature branch
stem branch feature-login
stem switch feature-login

# Make changes, add files
stem add src/login.c
stem commit -m "Implement login system"

# Switch back to main
stem switch main

# Merge feature
stem merge feature-login

# Push to cloud
stem push
```

#### Example 2: Bug Fix Branch
```cmd
# Create bugfix branch
stem switch -c bugfix-123

# Fix the bug
# ... edit files ...

# Commit fix
stem add .
stem commit -m "Fix issue #123"

# Merge back
stem switch main
stem merge bugfix-123

# Delete old branch
stem branch -d bugfix-123
```

#### Example 3: View History
```cmd
# See recent changes
stem history -n5

# Check what changed
stem diff

# View specific file changes
stem diff src/main.c
```

---

## Google Drive Integration

### First-Time Setup

1. **Run authorization:**
   ```cmd
   powershell -ExecutionPolicy Bypass -File gdrive_upload.ps1 -SetupAuth
   ```

2. **Browser opens automatically:**
   - Sign in to your Google account
   - Click "Allow" to grant permissions
   - Browser shows success message

3. **Token saved:**
   - Stored in `%USERPROFILE%\.stem_gdrive_token`
   - Valid for future pushes
   - No need to authorize again

### How It Works

1. **OAuth2 Authentication**
   - One-time authorization
   - Secure token storage
   - Automatic token refresh

2. **Upload Process (Compressed Archive)**
   - **Fast & Efficient**: Compresses entire project into single .zip archive
   - **Smart Compression**: Typically 30-70% size reduction
   - **Single Upload**: One file upload instead of thousands
   - **Speed**: 10-100x faster than uploading individual files
   - Creates timestamped backup: `stem_backup_YYYYMMDD_HHMMSS.zip`
   - Shows compression ratio and upload speed

3. **Download Process (Pull)**
   - Downloads latest backup archive
   - Extracts all files with directory structure intact
   - Fast restoration of entire project

4. **What Gets Uploaded**
   - Your actual project files (compressed)
   - Complete folder hierarchy
   - Everything except `.stem/` and `.stemignore` patterns
   - Original file structure preserved in archive

### Sharing with Others

To let others clone your project:

1. **Share your Google Drive folder:**
   - Right-click folder → Share
   - Add email addresses
   - Give "Viewer" or "Editor" access

2. **Share the folder URL:**
   ```
   https://drive.google.com/drive/folders/YOUR-FOLDER-ID
   ```

3. **Others can clone:**
   ```cmd
   stem clone https://drive.google.com/drive/folders/YOUR-FOLDER-ID
   ```
   *(Note: Clone feature is planned)*

---

## Working with .stemignore

### What is .stemignore?

A `.stemignore` file tells STEM which files and folders to exclude from commits and pushes. This prevents uploading:
- Build artifacts (*.exe, *.o, *.dll)
- Temporary files (*.tmp, *.log)
- IDE configuration (.vscode/, .idea/)
- Dependencies (node_modules/, vendor/)
- Test files

### Creating .stemignore

When you run `stem add .` for the first time, STEM offers to create one:

```
Do you want to create a .stemignore file now? [Y/n]: y
```

Or manually:
1. Copy `.stemignore.template` to your project
2. Rename to `.stemignore`
3. Customize for your needs

### Pattern Syntax

#### Directory Patterns (end with `/`)
```
bin/                  # Exclude bin/ directory
node_modules/         # Exclude node_modules/
.vscode/              # Exclude .vscode/
```

#### Extension Patterns (start with `*`)
```
*.exe                 # Exclude all .exe files
*.o                   # Exclude all .o files
*.log                 # Exclude all .log files
*.tmp                 # Exclude all temporary files
```

#### Exact Filename
```
config.local.json     # Exclude this specific file
.DS_Store             # Exclude macOS system file
Thumbs.db             # Exclude Windows system file
```

#### Comments
```
# This is a comment
# Use # at the start of a line
```

### Common .stemignore Template

```
# Build Outputs
bin/
obj/
build/
*.exe
*.dll
*.so
*.o
*.obj

# IDE Files
.vscode/
.vs/
.idea/
.kiro/

# Temporary Files
*.tmp
*.log
*.cache
*.bak
*~

# Dependencies
node_modules/
vendor/
packages/

# OS Files
.DS_Store
Thumbs.db
desktop.ini

# Test Files
test/
tests/
*.test
```

### Tips

- **Start restrictive, then relax** - Exclude too much rather than too little
- **Test before pushing** - Run `stem add .` to see what gets staged
- **Keep it updated** - Add new patterns as your project grows
- **Comment generously** - Explain why you're excluding things

---

## Best Practices

### 1. Use Meaningful Commit Messages

✅ **Good:**
```cmd
stem commit -m "Add user authentication feature"
stem commit -m "Fix memory leak in parser"
stem commit -m "Update README with installation steps"
```

❌ **Bad:**
```cmd
stem commit -m "changes"
stem commit -m "stuff"
stem commit -m "update"
```

### 2. Commit Often, Push Regularly

- Commit after completing a feature or fix
- Push at end of day or after major changes
- Don't wait too long between backups

### 3. Keep .stemignore Updated

- Review what gets staged before committing
- Add new patterns as needed
- Exclude large binary files

### 4. Organize Your Project

```
MyProject/
├── .stem/              # STEM repository data
├── .stemignore         # Ignore patterns
├── README.md           # Project overview
├── src/                # Source code
├── docs/               # Documentation
├── tests/              # Test files (excluded)
└── bin/                # Build outputs (excluded)
```

### 5. Document Your Setup

Add to your project README:
- How to initialize STEM
- What's in `.stemignore`
- How to restore from Google Drive

---

## Troubleshooting

### "stem: command not found"

**Cause:** STEM not in PATH

**Solutions:**
1. Restart your terminal
2. Run `INSTALL.bat` again
3. Manually add `C:\path\to\STEM_VCS\bin` to PATH

---

### "No authorization found"

**Cause:** Google Drive not set up

**Solution:**
```cmd
powershell -ExecutionPolicy Bypass -File C:\path\to\STEM_VCS\gdrive_upload.ps1 -SetupAuth
```

---

### "Failed to refresh access token"

**Cause:** Token expired or invalid

**Solution:**
Re-authorize:
```cmd
powershell -ExecutionPolicy Bypass -File C:\path\to\STEM_VCS\gdrive_upload.ps1 -SetupAuth
```

---

### Upload is Very Slow

**Causes:**
- Large files
- Many small files
- Slow internet

**Solutions:**
1. **Check file count:**
   ```cmd
   stem add .
   ```
   If thousands of files, improve `.stemignore`

2. **Exclude large files:**
   ```
   *.mp4
   *.zip
   *.iso
   ```

3. **Use compression** (future feature)

---

### "No files staged for commit"

**Cause:** Nothing added to staging area

**Solution:**
```cmd
stem add .
stem commit -m "Your message"
```

---

### Files Not Being Ignored

**Causes:**
- `.stemignore` syntax error
- `.stemignore` not in project root
- Pattern doesn't match

**Solutions:**
1. **Check .stemignore location:**
   ```cmd
   dir .stemignore
   ```
   Should be in project root

2. **Test patterns:**
   ```cmd
   stem add .
   ```
   See what gets staged

3. **Check syntax:**
   - Directories need trailing `/`
   - Extensions need leading `*`
   - No spaces around patterns

---

## FAQ

### Q: How is STEM different from Git?

**A:** STEM is simpler with cloud integration built-in:
- **Simpler:** Fewer commands, easier learning curve
- **Cloud-first:** Google Drive integration out of the box
- **No remotes complexity:** Just one Google Drive folder
- **Smaller projects:** Best for personal/small team use

Git is more powerful for large teams and complex workflows.

---

### Q: Can I use STEM with Git?

**A:** Not recommended. Choose one:
- **Use STEM:** For simple projects with cloud backup
- **Use Git:** For team projects with branching/merging

They store data differently and will conflict.

---

### Q: Where is my data stored?

**Locally:**
- `.stem/` folder in your project
- `.stem/objects/` contains committed files
- `.stem/index` tracks staged files

**Cloud:**
- Google Drive folder you specify
- Timestamped backup folders
- Full project structure preserved

---

### Q: What happens if I lose my local `.stem` folder?

**A:** Your actual project files are safe, but:
- You lose commit history
- Need to re-initialize: `stem init`
- Re-add files: `stem add .`
- Commit and push again

Your cloud backups remain intact.

---

### Q: Can multiple people work on the same project?

**A:** Limited support:
- ✅ Share Google Drive folder
- ✅ Others can download latest version
- ❌ No automatic merging of changes
- ❌ Last push wins (overwrites)

**Recommendation:** Use Git for team collaboration.

---

### Q: How much does Google Drive storage cost?

**A:** 
- **Free:** 15 GB
- **Paid:** 100 GB ($1.99/month), 200 GB ($2.99/month), etc.

STEM is efficient - excludes build artifacts automatically.

---

### Q: Is my data secure?

**A:** Yes:
- OAuth2 authentication (no passwords stored)
- HTTPS for all uploads
- Google's security infrastructure
- Only you control access to your Drive folder

---

### Q: Can I push to multiple Google Drive accounts?

**A:** Not currently supported. You can:
1. Share one Drive folder with multiple accounts
2. Or manually change remotes

---

### Q: Does STEM work offline?

**A:** Partially:
- ✅ `stem init`, `stem add`, `stem commit` work offline
- ❌ `stem push` requires internet (uploads to cloud)

---

### Q: What's the maximum file size?

**A:** 
- **STEM:** No hard limit
- **Google Drive:** 5 TB per file (web upload), unlimited (Drive API)

In practice, avoid files over 1 GB for performance.

---

## Support & Contributing

### Get Help

- 📖 **Documentation:** This manual
- 🐛 **Issues:** Report bugs on GitHub (link TBD)
- 💬 **Community:** (forum/discord TBD)

### Report a Bug

1. Check [Troubleshooting](#troubleshooting) first
2. Collect information:
   - `stem --version`
   - Error message
   - Steps to reproduce
3. Report on GitHub Issues

### Contributing

STEM VCS is open source! Contributions welcome:
- 🐛 Bug fixes
- ✨ New features
- 📝 Documentation improvements
- 🧪 Tests

See `CONTRIBUTING.md` for guidelines.

---

## License

STEM VCS is released under the MIT License.
See `LICENSE` file for full text.

---

## Changelog

### Version 1.0.0 (2026)

**Initial Release:**
- ✅ Core VCS functionality (init, add, commit, push)
- ✅ Google Drive integration with OAuth2
- ✅ .stemignore support
- ✅ Directory structure preservation
- ✅ Real-time upload progress
- ✅ Windows native implementation in C
- ✅ Complete installation wizard
- ✅ Comprehensive documentation

---

**Last Updated:** August 3, 2026  
**STEM VCS Version:** 1.0.0  
**Copyright © 2026 STEM VCS Contributors**
