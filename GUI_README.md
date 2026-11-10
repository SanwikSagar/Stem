# STEM VCS - Web GUI

**Modern, cross-platform graphical interface for STEM VCS**

---

## 🎨 Overview

The STEM VCS Web GUI is a modern, responsive web-based interface that runs locally on your machine. It provides a beautiful, easy-to-use graphical interface for all STEM VCS operations.

### Why Web-Based?

- **Truly Cross-Platform** - Works on Windows, macOS, and Linux
- **Modern UI** - Beautiful, responsive design
- **Easy to Maintain** - Standard web technologies
- **No Dependencies** - Just compile and run
- **Fast Development** - Quick to implement and update

---

## 🚀 Quick Start

### Windows

```cmd
build_gui.bat
bin\stem_gui.exe
```

### macOS / Linux

```bash
make gui
./bin/stem_gui
```

The server will automatically open your default browser to `http://localhost:8765`

---

## 📦 Building

### Using Build Script (Windows)

```cmd
build_gui.bat
```

### Using Makefile (All Platforms)

```bash
make gui
```

### Using CMake

```bash
cmake -B build -DBUILD_GUI_WEB=ON
cmake --build build
```

### Manual Compilation

```bash
# Compile core library
gcc -c -O2 -Wall -Wextra -Iinclude src/core/*.c

# Compile GUI server
gcc -c -O2 -Wall -Wextra -Iinclude src/gui_web/server.c

# Link (Windows)
gcc -o bin/stem_gui.exe *.o -lws2_32

# Link (Unix)
gcc -o bin/stem_gui *.o
```

---

## 🎯 Features

### Dashboard
- Repository overview
- Quick stats (staged, modified, total files)
- Recent activity feed
- At-a-glance status

### Changes View
- File list with status indicators
- Staged/unstaged visualization
- File preview
- Quick staging/unstaging

### History View
- Commit timeline
- Commit messages and metadata
- Author and date information
- Commit hash display

### Branches View
- List all branches
- Current branch indicator
- Switch branches
- Create new branches

### Remotes View
- Cloud storage configuration
- Connection status
- Remote URL management
- Test connection

### Settings
- User configuration
- Email settings
- Repository preferences

---

## 🖥️ Interface

### Layout

```
┌──────────────────────────────────────────────────┐
│ Top Bar (Logo, Branch, Actions)                  │
├────────┬─────────────────────────────┬───────────┤
│        │                             │           │
│  Left  │      Main Content           │   Right   │
│  Nav   │      (Dashboard/Changes     │   Status  │
│        │       /History/etc.)        │   Panel   │
│        │                             │           │
├────────┴─────────────────────────────┴───────────┤
│ Bottom Panel (Commit Message)                    │
├──────────────────────────────────────────────────┤
│ Status Bar                                       │
└──────────────────────────────────────────────────┘
```

### Color Scheme

**Dark Theme** (Professional & Modern)
- Background: `rgb(10, 12, 16)`
- Panels: `rgb(17, 21, 28)`
- Accent: `rgb(59, 130, 246)` (Blue)
- Success: `rgb(16, 185, 129)` (Green)
- Warning: `rgb(245, 158, 11)` (Amber)
- Error: `rgb(239, 68, 68)` (Red)

---

## 🔌 API Endpoints

The GUI server provides a REST API for the web interface:

### Status
- `GET /api/status` - Get repository status

### Files
- `GET /api/files` - List all files
- `POST /api/stage` - Stage files
- `POST /api/unstage` - Unstage files

### Commits
- `GET /api/history` - Get commit history
- `POST /api/commit` - Create a commit

### Cloud Sync
- `POST /api/push` - Push to cloud
- `POST /api/pull` - Pull from cloud
- `POST /api/sync` - Bidirectional sync

### Branches
- `GET /api/branches` - List branches
- `POST /api/branch/create` - Create branch
- `POST /api/branch/switch` - Switch branch

---

## 🛠️ Technical Details

### Server

- **Language:** C
- **Port:** 8765 (configurable)
- **Protocol:** HTTP/1.1
- **Threading:** Single-threaded (sufficient for local use)

### Frontend

- **HTML5** - Semantic markup
- **CSS3** - Modern styling with CSS variables
- **JavaScript** - Vanilla JS (no frameworks)

### File Structure

```
src/gui_web/
└── server.c          # Web server implementation

gui_web/
├── index.html        # Main HTML file
├── styles.css        # Styles
└── app.js            # JavaScript logic
```

---

## 🎨 Customization

### Changing the Port

Edit `server.c`:
```c
#define PORT 8765  // Change to your preferred port
```

### Modifying Colors

Edit `styles.css` CSS variables:
```css
:root {
    --color-primary: rgb(59, 130, 246);
    --color-success: rgb(16, 185, 129);
    /* ... */
}
```

---

## 🚀 Usage

### Starting the Server

```bash
# Start server
./bin/stem_gui

# Output:
# ========================================
#   STEM VCS - Web GUI Server
# ========================================
# 
# [OK] Server running on http://localhost:8765
# [OK] Press Ctrl+C to stop
```

### Accessing the GUI

Open your browser and navigate to:
```
http://localhost:8765
```

Or use localhost alternative:
```
http://127.0.0.1:8765
```

### Stopping the Server

Press `Ctrl+C` in the terminal where the server is running.

---

## 📱 Browser Compatibility

**Tested Browsers:**
- Chrome/Edge 90+
- Firefox 88+
- Safari 14+
- Opera 76+

**Requirements:**
- JavaScript enabled
- CSS Grid support
- Modern CSS support

---

## 🔒 Security

### Local Only

The server binds to `localhost` only and is not accessible from external machines. This is intentional for security.

### No Authentication

Since the server is local-only, no authentication is required.

### CORS

CORS headers are enabled for local development.

---

## 🐛 Troubleshooting

### Port Already in Use

**Error:** `Failed to bind to port 8765`

**Solution:** Either stop the other application using port 8765, or change the port in `server.c`

### Browser Doesn't Open Automatically

**Solution:** Manually open your browser and navigate to `http://localhost:8765`

### Files Not Found (404)

**Solution:** Ensure you're running the server from the STEM VCS root directory

### Cannot Connect

**Solution:**
1. Verify the server is running
2. Check firewall settings
3. Try `http://127.0.0.1:8765` instead

---

## 🎯 Keyboard Shortcuts

(Planned for future versions)

- `Ctrl+K` - Quick search
- `Ctrl+S` - Stage all
- `Ctrl+Enter` - Create commit
- `Ctrl+P` - Push
- `Ctrl+Shift+P` - Pull

---

## 📊 Performance

- **Server Memory:** ~5MB
- **Startup Time:** < 100ms
- **Response Time:** < 10ms (local)
- **File Limit:** 10,000+ files

---

## 🔮 Future Enhancements

### Planned Features
- [ ] Real-time file watching
- [ ] Diff viewer with syntax highlighting
- [ ] Merge conflict resolution UI
- [ ] Visual branch graph
- [ ] Drag-and-drop file staging
- [ ] Dark/Light theme toggle
- [ ] Keyboard shortcuts
- [ ] Search functionality

### Nice to Have
- [ ] Multi-language support
- [ ] Customizable themes
- [ ] Plugin system
- [ ] Terminal integration
- [ ] Git repository import

---

## 🤝 Contributing

Want to improve the GUI?

**Easy Contributions:**
- CSS tweaks and styling
- New icons and graphics
- Responsive design improvements
- Accessibility enhancements

**Advanced Contributions:**
- API endpoint implementations
- Real-time updates (WebSockets)
- Advanced features
- Performance optimizations

---

## 📄 License

MIT License - Same as STEM VCS

---

## 🆘 Support

**Issues:**
- Check this README
- Run `stem doctor` in CLI
- Check browser console for errors

**Questions:**
- See main [README.md](README.md)
- See [MANUAL.md](MANUAL.md)

---

**STEM VCS v1.0.0**  
**Web GUI - Modern Interface for Modern Development**

---

*Made with ❤️ for developers who appreciate good UX*
