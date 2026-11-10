# STEM VCS - Project Structure

**Clean, organized, production-ready project layout**

---

## Core Files (Root Directory)

| File | Purpose |
|------|---------|
| **INSTALL.bat** | ⭐ Single complete installation script |
| **MANUAL.md** | ⭐ Complete documentation (all-in-one) |
| **README.md** | Quick overview and links |
| **LICENSE** | MIT License |
| **.stemignore.template** | Template for ignoring files |

---

## Installation

**For users:**
```cmd
INSTALL.bat
```

**What it does:**
1. Checks prerequisites (GCC, PowerShell)
2. Builds STEM CLI
3. Adds to system PATH
4. Sets up Google Drive (optional)
5. Verifies installation

---

## Documentation

**For users:**
- Start with `README.md` (quick overview)
- Read `MANUAL.md` for everything else

**MANUAL.md covers:**
- Complete installation guide
- All commands with examples
- Google Drive setup walkthrough
- .stemignore patterns
- Best practices
- Troubleshooting
- FAQ

---

## Archived Files

Old documentation moved to `docs/archive/`:
- README.txt
- READ_ME_FIRST.txt
- START_HERE.md  
- START_HERE_SIMPLE.txt
- IMPLEMENTATION_SUMMARY.md
- SETUP.bat (old)
- START.bat (old)

**Why archived:**
- Multiple overlapping docs confused users
- Now have single source of truth (MANUAL.md)
- Old setup scripts replaced by INSTALL.bat

---

## Directory Structure

```
STEM_VCS/
│
├── 📄 INSTALL.bat              ⭐ Run this first
├── 📄 MANUAL.md                ⭐ Complete documentation
├── 📄 README.md                Quick overview
├── 📄 LICENSE                  MIT License
├── 📄 .stemignore.template     Template for projects
│
├── 📁 bin/                     Compiled executables
│   └── stem.exe
│
├── 📁 src/                     Source code
│   ├── core/                   Core library
│   └── cli/                    CLI interface
│
├── 📁 include/                 Header files
│   └── stem/                   Public API
│
├── 📁 obj/                     Object files (build)
│
├── 📁 tools/                   MinGW compiler
│   └── mingw64/
│
├── 📁 docs/                    Documentation
│   └── archive/                Old docs (archived)
│
├── 📄 gdrive_upload.ps1        Google Drive script
├── 📄 AddToPath.ps1            PATH helper
├── 📄 build_cli.bat            Build script
├── 📄 build_gui.bat            GUI build script
└── 📄 CMakeLists.txt           CMake configuration
```

---

## For New Users

**What to read:**
1. README.md (5 min) - Overview
2. INSTALL.bat (run it) - Installation
3. MANUAL.md (reference) - When you need details

**What to run:**
```cmd
# Install
INSTALL.bat

# Use STEM
cd your-project
stem init
stem add .
stem commit -m "First commit"
stem push
```

---

## For Developers

**Build from source:**
```cmd
build_cli.bat
```

**Files to modify:**
- `src/cli/*.c` - CLI commands
- `src/core/*.c` - Core library
- `include/stem/*.h` - Public API
- `gdrive_upload.ps1` - Cloud upload logic

---

## Key Improvements

### Before (Messy)
- ❌ 10+ documentation files
- ❌ 5+ setup scripts
- ❌ Confusing entry points
- ❌ Scattered information
- ❌ Hard to maintain

### After (Clean)
- ✅ 1 installation script (INSTALL.bat)
- ✅ 1 complete manual (MANUAL.md)
- ✅ 1 quick reference (README.md)
- ✅ Clear structure
- ✅ Easy to maintain

---

## Maintenance

**When adding features:**
1. Update code in `src/`
2. Update MANUAL.md (command reference)
3. Update README.md if major feature
4. Update INSTALL.bat if setup changes

**Keep these in sync:**
- Version numbers
- Command examples
- Feature lists

---

**Last Updated:** August 3, 2026  
**STEM VCS Version:** 1.0.0
