# STEM VCS GUI - Complete Production Guide

## Overview
The STEM VCS GUI is a professional, production-ready web interface for STEM Version Control System. It provides a modern VSCode-like experience with full integration to the STEM VCS backend.

## Architecture

### Frontend (gui_web/)
- **index.html** - Main HTML structure with modals
- **styles.css** - Professional VSCode-inspired dark theme
- **app.js** - Complete application logic with real API integration

### Backend (src/gui_web/server_real.c)
- Embedded HTTP server on port 8765
- Real-time file system scanning
- Direct .stem/ repository file manipulation
- REST API for all VCS operations

## Features Implemented

### ✅ Repository Overview
- Dashboard with statistics (staged files, modified files, commits)
- Recent commit history preview
- Quick status indicators

### ✅ File Management
- Real-time file scanning with status tracking
- Three categories: Staged, Modified, Untracked
- File icons based on extension
- File size display
- Addition/deletion stats per file
- Limit to 20 untracked files displayed (performance)

### ✅ Commit Operations
- Stage All / Unstage All buttons
- Professional modal dialog for commit messages
- Real commit creation with timestamp-based hashing
- Commit history updates automatically
- Keyboard shortcuts: Ctrl+Enter to commit, Escape to cancel

### ✅ Commit History
- Beautiful tree visualization with connecting lines
- Commit graph with branches
- Detailed commit information:
  - Commit message
  - Author
  - Date/time
  - Hash (short)
  - File count
  - Additions/deletions
- Hover actions on commits
- Click to view commit details

### ✅ Branch Management
- Branch selector dropdown in titlebar
- Branch switching (updates .stem/HEAD)
- Create new branch via modal dialog
- Branch validation (alphanumeric, /, _, -)
- Current branch indicator
- Branches view showing all branches

### ✅ Remote Storage
- Configure Google Drive URL via modal
- Connection status indicator
- Storage statistics (only if available from API)
- Change remote at any time
- Persistent storage in .stem/remote.txt

### ✅ Sync Operations
- Pull from remote
- Push to remote
- Sync (pull + push)
- Integration with gdrive_upload.ps1

### ✅ Professional UI/UX
- Modal dialogs for all user inputs (no browser popups)
- Keyboard shortcuts:
  - Ctrl+K: Focus search
  - Escape: Close modals
  - Enter: Submit in single-line inputs
  - Ctrl+Enter: Submit commit message
- Hover effects and transitions
- Status bar with real-time feedback
- Auto-refresh every 30 seconds
- Responsive design

## API Endpoints

### GET Endpoints
```
GET /api/status       - Repository status and current branch
GET /api/files        - All tracked/untracked files with status
GET /api/history      - Commit log from .stem/commits.log
GET /api/branches     - All branches from .stem/branches.txt
GET /api/remote       - Remote configuration from .stem/remote.txt
```

### POST Endpoints
```
POST /api/stage       - Stage all files
POST /api/unstage     - Unstage all files
POST /api/commit      - Create new commit with message
POST /api/push        - Push to remote (calls PowerShell script)
POST /api/pull        - Pull from remote (calls PowerShell script)
POST /api/remote      - Update remote URL
POST /api/branch/switch - Switch to different branch
POST /api/branch/create - Create new branch
```

## File Structure

```
.stem/
├── HEAD              - Current branch reference
├── commits.log       - Commit history (hash|date|files|message)
├── branches.txt      - Branch list (name|)
├── staging.txt       - Staged files (one per line)
├── remote.txt        - Remote URL
└── refs/
    └── heads/
        ├── main      - Branch reference (commit hash)
        └── ...       - Other branches
```

## How to Build

### Windows (MinGW)
```batch
build_gui.bat
```

### Linux/macOS
```bash
make gui
```

## How to Run

1. Navigate to your STEM repository
2. Run the GUI server:
   ```
   bin\stem_gui.exe        # Windows
   ./bin/stem_gui          # Linux/macOS
   ```
3. Browser opens automatically to http://localhost:8765
4. Use the GUI to manage your repository

## Usage Guide

### First Time Setup
1. Launch GUI in your project directory
2. Click "Change Remote" to configure Google Drive
3. Enter your Google Drive folder URL
4. Click Save

### Daily Workflow
1. Make changes to your files
2. Click "Changes" in sidebar to see modified files
3. Click "Stage All" to stage changes
4. Click "Commit" and enter a message
5. Click "Push" to upload to remote
6. Use "Sync" for quick pull+push

### Branch Management
1. View current branch in titlebar dropdown
2. Click "Branch" to create a new branch
3. Enter branch name (e.g., feature/new-feature)
4. Switch branches using the dropdown
5. Branches view shows all branches with indicators

### Viewing History
1. Click "History" in sidebar
2. See commit tree visualization
3. Hover over commits for actions
4. Click commits to view details

## Keyboard Shortcuts

| Shortcut | Action |
|----------|--------|
| Ctrl+K | Focus search box |
| Escape | Close any modal |
| Enter | Submit (Remote URL, Branch name) |
| Ctrl+Enter | Submit commit message |

## Technical Details

### Performance Optimizations
- File scanning limited to non-.stem directories
- Commit history buffered to prevent memory issues
- Untracked file display capped at 20 items
- 30-second auto-refresh to balance UX and performance

### Security
- Server binds to localhost only (not exposed to network)
- Content-Length header validation
- Multi-packet POST body handling
- Path validation for file operations

### Cross-Platform Support
- Windows: MinGW build with Winsock2
- Linux: Standard BSD sockets
- macOS: BSD sockets with platform-specific file operations
- Browser auto-launch on all platforms

## Troubleshooting

### GUI won't start
- Ensure port 8765 is not in use
- Check you're in a valid STEM repository (has .stem/ directory)
- Verify stem_gui.exe compiled successfully

### Files not showing
- Refresh the page (F5)
- Check .stem/staging.txt exists
- Verify file permissions

### Branch switching not working
- Ensure branch exists in .stem/refs/heads/
- Check .stem/HEAD is writable
- Rebuild GUI with latest code

### Remote not updating
- Check .stem/remote.txt is writable
- Verify URL format (https://drive.google.com/...)
- Ensure no trailing whitespace in URL

## Future Enhancements (Not Yet Implemented)

- Individual file staging/unstaging
- File diff preview
- Merge conflict resolution UI
- Tag management
- Search in commits
- Theme toggle (light/dark)
- Back/forward navigation
- Commit details side panel
- Branch merge UI
- Settings configuration panel

## Code Quality

- Zero compilation errors (clean build)
- Minimal warnings (only string truncation safeguards)
- Professional code structure
- Memory management (malloc/free balanced)
- Error handling throughout
- Consistent naming conventions
- Comprehensive comments

## Design Philosophy

The GUI was designed to:
1. Look professional (no AI-generated appearance)
2. Use real integrations (no demo data in production)
3. Match VSCode aesthetic (familiar to developers)
4. Be production-ready (no placeholders or TODOs)
5. Work seamlessly with OS (proper file handling)
6. Provide immediate feedback (status messages)
7. Support keyboard workflows (shortcuts)
8. Be responsive and fast (efficient rendering)

## License

This GUI is part of STEM VCS and shares the same MIT license.
Copyright (c) 2026 STEM VCS Contributors

---

**Version:** 1.0.0  
**Last Updated:** 2026-08-05  
**Status:** Production Ready ✅
