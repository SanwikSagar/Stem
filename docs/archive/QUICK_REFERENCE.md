# 🚀 STEM VCS - Quick Reference Guide

## Essential Commands (Top 10)

### 1. Initialize Repository
```bash
stem init
```
Creates a new STEM repository in the current directory.

### 2. Check Status
```bash
stem status
```
Shows current branch, staged files, and working tree status.

### 3. Stage Files
```bash
stem add file.txt       # Add specific file
stem add .              # Add all files
stem add *.js           # Add by pattern
```

### 4. Create Commit
```bash
stem commit -m "Your commit message"
```

### 5. View History
```bash
stem history            # Show all commits
stem history -n 10      # Last 10 commits
```

### 6. Add Remote Storage
```bash
stem remote add cloud gdrive://MyDrive/Projects/MyRepo
stem remote add cloud dropbox://Projects/MyRepo
stem remote add cloud s3://my-bucket/myrepo
```

### 7. Push Changes
```bash
stem push               # Push to default remote
stem push cloud         # Push to specific remote
```

### 8. Pull Changes
```bash
stem pull               # Pull from default remote
stem pull cloud         # Pull from specific remote
```

### 9. Create Branch
```bash
stem branch feature     # Create new branch
stem branch -l          # List branches
```

### 10. Switch Branch
```bash
stem switch feature     # Switch to branch
stem switch main        # Back to main
```

---

## GUI Quick Guide

### Launch GUI
```bash
.\bin\stem-gui.exe
```
Or double-click `stem-gui.exe` in File Explorer.

### Main Interface

**Top Bar**
- New Project, Open, Clone buttons
- Quick actions toolbar

**Left Sidebar**
- Dashboard - Overview
- Changes - View modifications
- Commits - Browse history
- Branches - Manage branches
- Remotes - Configure storage
- Settings - Preferences

**Main Workspace**
- File browser
- Diff viewer
- Commit details

**Right Sidebar**
- Commit information
- File details
- Statistics

**Bottom Panel**
- Commit message input
- Stage/Unstage controls
- Commit button

**Status Bar**
- Current branch
- Repository status
- Cloud sync indicator

---

## Common Workflows

### Starting a New Project
```bash
# 1. Initialize
stem init

# 2. Add cloud storage
stem remote add cloud gdrive://MyDrive/Projects/MyProject

# 3. Add files
stem add .

# 4. First commit
stem commit -m "Initial commit"

# 5. Push to cloud
stem push
```

### Daily Development
```bash
# 1. Make changes to files

# 2. Check what changed
stem status

# 3. Stage changes
stem add .

# 4. Commit
stem commit -m "Add new feature"

# 5. Push
stem push
```

### Working with Branches
```bash
# Create feature branch
stem branch feature-login

# Switch to it
stem switch feature-login

# Make changes and commit
stem add .
stem commit -m "Add login feature"

# Switch back to main
stem switch main

# Merge feature
stem merge feature-login
```

---

## Remote Storage URLs

### Google Drive
```
gdrive://MyDrive/Projects/MyRepo
gdrive://Shared/TeamProjects/MyRepo
```

### Dropbox
```
dropbox://Projects/MyRepo
dropbox://Team/MyRepo
```

### Amazon S3
```
s3://my-bucket/myrepo
s3://my-bucket/path/to/repo
```

### OneDrive
```
onedrive://Documents/Projects/MyRepo
onedrive://Shared/MyRepo
```

---

## Useful Options

### Status
```bash
stem status -s          # Short format
stem status -v          # Verbose
```

### History
```bash
stem history -n 5       # Last 5 commits
stem history --graph    # Visual graph
```

### Add
```bash
stem add -A             # All changes
stem add -u             # Updated files only
```

### Commit
```bash
stem commit -m "msg"    # With message
stem commit -a          # Stage all and commit
```

### Remote
```bash
stem remote list        # List all remotes
stem remote show cloud  # Show details
stem remote remove old  # Remove remote
```

---

## Troubleshooting

### Check Repository Health
```bash
stem doctor
```

### Verify Objects
```bash
stem verify
```

### View Configuration
```bash
stem config --list
```

### Get Help
```bash
stem help               # General help
stem help commit        # Command-specific help
```

---

## Tips & Tricks

### 1. Commit Often
Make small, focused commits with clear messages.

### 2. Use Descriptive Messages
```bash
# Good
stem commit -m "Fix login validation bug"

# Bad
stem commit -m "Fixed stuff"
```

### 3. Check Status Before Committing
```bash
stem status
stem add .
stem commit -m "Your message"
```

### 4. Use Branches for Features
Keep main branch stable, develop in feature branches.

### 5. Push Regularly
Don't let too many commits pile up before pushing.

---

## Keyboard Shortcuts (GUI)

| Key | Action |
|-----|--------|
| `Ctrl+N` | New Project |
| `Ctrl+O` | Open Project |
| `Ctrl+S` | Stage All |
| `Ctrl+Enter` | Commit |
| `F5` | Refresh |
| `Ctrl+P` | Push |
| `Ctrl+L` | Pull |
| `Ctrl+B` | Branches |
| `Ctrl+,` | Settings |

---

## File Patterns

### Ignore Files
Create `.stemignore` file:
```
*.log
*.tmp
node_modules/
build/
.env
```

### Include Patterns
```bash
stem add *.js          # All JavaScript files
stem add src/**        # All in src directory
stem add *.{js,ts}     # Multiple extensions
```

---

## Exit Codes

| Code | Meaning |
|------|---------|
| 0 | Success |
| 1 | General error |
| 2 | Invalid usage |
| 3 | Repository not found |
| 4 | Network error |

---

## Configuration

### User Settings
```bash
stem config user.name "Your Name"
stem config user.email "you@example.com"
```

### Repository Settings
```bash
stem config core.editor "notepad"
stem config color.ui auto
```

### View All Settings
```bash
stem config --list
```

---

## Quick Checks

### Is this a STEM repo?
```bash
dir .stem            # Should exist
```

### What branch am I on?
```bash
stem status          # Shows current branch
```

### What remotes do I have?
```bash
stem remote list
```

### What's in the staging area?
```bash
stem status -s       # Shows staged files
```

---

## Learn More

- **USER_MANUAL.md** - Complete documentation
- **QUICKSTART.md** - 5-minute tutorial
- **ARCHITECTURE.md** - Technical details
- **START_HERE.md** - Getting started guide

---

**Need help?** Run `stem help` or open **USER_MANUAL.md**

*STEM VCS - Own your code, own your history, own your storage*

