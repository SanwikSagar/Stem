# STEM VCS - Web GUI Quick Start

## Launch the GUI

### Windows
```bash
cd C:\path\to\STEM_VCS
.\bin\stem_gui.exe
```

Your browser will automatically open to: **http://localhost:8765**

### Linux/Mac
```bash
cd /path/to/STEM_VCS
./bin/stem_gui
```

## First-Time Setup

### 1. Initialize Repository (if not done)
If you don't have a `.stem` folder yet:
```bash
stem init
```

### 2. Configure Remote (Optional)
In the GUI:
1. Click the **"Change Remote"** button in the right panel
2. Enter your Google Drive folder URL
3. Click **"Save"**

Or via CLI:
```bash
stem remote set https://drive.google.com/drive/folders/YOUR_FOLDER_ID
```

## Basic Workflow

### Make Changes & Commit

1. **View Changes**: Click "Changes" in the sidebar
   - See all modified, staged, and untracked files
   
2. **Stage Files**: 
   - Click **"Stage All"** to stage everything
   - Or click individual files to stage them
   
3. **Commit**:
   - Click the **"Commit"** button
   - Enter your commit message in the modal
   - Press **Ctrl+Enter** or click **"Commit"**

4. **Push to Cloud**:
   - Click **"Push"** in the top toolbar
   - Or use **"Sync"** to pull and push in one action

### Work with Branches

1. **Create Branch**:
   - Click **"Branch"** button in top toolbar
   - Enter branch name (e.g., `feature/new-feature`)
   - Press Enter or click **"Create Branch"**

2. **Switch Branch**:
   - Use the branch dropdown in the top-left
   - Select the branch you want

3. **Merge Branch**:
   - Click **"Merge"** button in top toolbar
   - Select source and target branches
   - Click **"Merge"**

4. **Delete Branch**:
   - Switch to a different branch first
   - Use the branch management modal
   - Click delete icon next to the branch

### Create Tags

1. Click **"Tags"** in the sidebar
2. Click **"Create Tag"**
3. Enter tag name (e.g., `v1.0.0`)
4. Tag is created at current commit

### Multiple Repositories

1. **Add Repository**:
   - Click the **"+"** button in the top toolbar
   - Enter repository name and path
   - Optionally set remote URL
   - Click **"Add Repository"**

2. **Switch Repository**:
   - Use the repository dropdown in the top-left
   - Select the repository you want
   - GUI will switch to that directory

## Keyboard Shortcuts

| Shortcut | Action |
|----------|--------|
| `Ctrl+K` | Focus search |
| `Escape` | Close any modal |
| `Enter` | Submit modal (branch, tag, remote) |
| `Ctrl+Enter` | Submit commit message |

## View Descriptions

### Overview
Dashboard showing:
- Staged files count
- Modified files count
- Total commits
- Recent commits

### Changes
Complete file status view:
- **Staged Changes**: Files ready to commit (green)
- **Modified Files**: Changed but not staged (yellow)
- **Untracked Files**: New files not in version control (gray)

### History
Visual commit timeline with:
- Commit graph with tree lines
- Commit message
- Author and date
- Files changed count
- Hash for reference

### Branches
List all branches with:
- Current branch indicator (*)
- Ahead/behind counts
- Branch operations (create, delete, merge)

### Tags
List all tags with:
- Tag name
- Associated commit
- Creation date
- Tag operations (create, delete)

### Remotes
Cloud storage configuration:
- Remote name and URL
- Connection status
- Storage usage (if available)
- Push/Pull/Sync buttons

### Settings
User configuration:
- User name and email
- Auto-stage preference
- Auto-sync settings
- Sync interval

## Status Bar

Bottom status bar shows:
- **Current branch**
- **↑ Ahead**: Commits not pushed
- **↓ Behind**: Commits not pulled
- **Working tree status**: Clean or X files changed
- **Status messages**: Real-time operation feedback

## Right Panel

Always-visible panel showing:
- **Remote info**: Connection and storage
- **Status**: Quick stats (staged, working tree, untracked)
- **Actions**: Quick access buttons
- **Recent Activity**: Last operations

## Tips

1. **Auto-refresh**: GUI refreshes every 30 seconds automatically
2. **Real-time updates**: Most operations update immediately
3. **No page reloads needed**: Everything is single-page
4. **Modal dialogs**: All inputs use professional modals (no browser popups!)
5. **Error messages**: Check status bar for operation results

## Troubleshooting

### GUI Won't Start
```bash
# Rebuild the GUI
.\build_gui.bat

# Try again
.\bin\stem_gui.exe
```

### Port Already in Use
Another instance may be running. Close it or change the port in `src/gui_web/server_real.c` (line 40):
```c
#define PORT 8765  // Change to different port
```

### Files Not Showing
Make sure you're in a STEM repository:
```bash
# Check for .stem folder
dir .stem

# If not present, initialize
stem init
```

### Remote Not Working
1. Check remote URL is correct
2. Verify Google Drive permissions
3. Test with CLI first: `stem push`

## CLI Commands Reference

While using the GUI, you can still use CLI:

```bash
# View status
stem status

# Stage files
stem add <file>

# Commit
stem commit "message"

# Push/Pull
stem push
stem pull

# Branch operations
stem branch feature-name      # Create
stem branch -d branch-name    # Delete

# Tag operations
stem tag v1.0.0              # Create
stem tag -d tag-name         # Delete

# View history
stem log
```

## Support

For issues or questions:
1. Check `MANUAL.md` for detailed documentation
2. See `README.md` for project overview
3. Review `GUI_FIX_COMPLETE.md` for recent changes

---

**Enjoy using STEM VCS!** 🌱
