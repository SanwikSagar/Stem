# STEM VCS - Quick Reference Card

**Essential commands at your fingertips**

---

## 🚀 Basic Workflow

```bash
stem init                          # Initialize repository
stem add .                         # Stage all files
stem add file.txt                  # Stage specific file
stem commit -m "message"           # Create commit
stem history                       # View commits
```

---

## 🌳 Branching

```bash
stem branch                        # List branches
stem branch feature                # Create branch
stem switch feature                # Switch to branch
stem branch -d old-branch          # Delete branch
```

---

## 🔄 Merging

```bash
stem merge feature                 # Merge branch
stem merge --abort                 # Abort merge
```

---

## ☁️ Cloud Sync

```bash
# Setup (one-time)
stem remote add cloud <gdrive-url>

# Sync operations
stem push                          # Upload to cloud
stem pull                          # Download from cloud
stem sync                          # Bidirectional sync
stem clone <url>                   # Clone from cloud
```

---

## 📊 Information

```bash
stem status                        # Repository status
stem history                       # Commit history
stem diff                          # Show changes
stem doctor                        # Run diagnostics
```

---

## 🔧 Configuration

```bash
# User settings
stem config user.name "Your Name"
stem config user.email "you@example.com"

# View settings
stem config --list                 # List all config
stem config --global user.name     # Get global setting
```

---

## 📝 History & Restore

```bash
stem history                       # View commits
stem history -n 10                 # Last 10 commits
stem diff                          # Show changes
stem diff file.txt                 # Changes in file
stem restore <commit-id> file.txt  # Restore specific file
stem verify <commit-id>            # Verify commit integrity
```

---

## 🔍 Advanced

```bash
# Commit options
stem commit -m "msg"               # Simple commit
stem commit --amend -m "new msg"   # Fix last commit message

# Branch options
stem branch -a                     # List all branches
stem branch -d name                # Delete branch
stem branch -D name                # Force delete

# Remote options
stem remote list                   # List remotes
stem remote remove cloud           # Remove remote
```

---

## 🆘 Help & Troubleshooting

```bash
stem help                          # Show all commands
stem --version                     # Show version
stem doctor                        # Run diagnostics
```

**Common Issues:**
- `stem doctor` - Diagnose problems
- Check `.stemignore` if files won't stage
- Use `stem status` to see repository state

---

## 📁 File Management

```bash
# Ignore patterns (.stemignore)
*.log                              # All log files
build/                             # Build directory
*.tmp                              # Temp files
node_modules/                      # Dependencies

# Wildcards
*.txt                              # All .txt files
temp*                              # Files starting with "temp"
**/test/**                         # All test directories
```

---

## 🎯 Keyboard Tips

**Windows:**
```cmd
stem status                        # Check what changed
stem add .                         # Stage everything
stem commit -m "msg"               # Commit
stem push                          # Backup
```

**Unix (macOS/Linux):**
```bash
stem status                        # Check what changed
stem add .                         # Stage everything
stem commit -m "msg"               # Commit
stem push                          # Backup
```

---

## 📊 Workflow Examples

### Daily Work
```bash
stem init                          # Once per project
stem add .                         # Stage changes
stem commit -m "Added feature X"   # Commit
stem push                          # Backup to cloud
```

### Feature Development
```bash
stem branch feature-x              # Create branch
stem switch feature-x              # Switch to it
# ... work on feature ...
stem add .
stem commit -m "Implement feature X"
stem switch main                   # Back to main
stem merge feature-x               # Merge feature
```

### Collaboration
```bash
stem clone <url>                   # Get project
stem pull                          # Get updates
# ... make changes ...
stem push                          # Share changes
```

---

## 🌐 Google Drive Setup

### Get Folder ID
1. Create folder in Google Drive
2. Open the folder
3. Copy URL: `https://drive.google.com/drive/folders/YOUR-ID-HERE`
4. Use the ID part after `folders/`

### First-Time Auth
```bash
stem remote add cloud <url>
stem push
# Browser opens → Click "Allow" → Done!
```

---

## 💡 Pro Tips

1. **Use `.stemignore`** - Exclude build artifacts automatically
2. **Frequent commits** - Commit often with descriptive messages
3. **Branch for features** - Keep main branch stable
4. **Regular backups** - `stem push` frequently
5. **Check before commit** - `stem status` and `stem diff`

---

## 🔗 Quick Links

- **Full Manual:** [MANUAL.md](MANUAL.md)
- **Getting Started:** [GETTING_STARTED.md](GETTING_STARTED.md)
- **Features:** [FEATURES.md](FEATURES.md)
- **Contributing:** [CONTRIBUTING.md](CONTRIBUTING.md)

---

**STEM VCS v1.0.0** | [GitHub](#) | [Website](#)

**Print this page for easy reference!**
