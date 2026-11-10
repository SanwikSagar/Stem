# Contributing to STEM VCS

Thank you for your interest in contributing to STEM VCS! This guide will help you get started.

---

## 🚀 Quick Start for Developers

### 1. Set Up Development Environment

#### Windows
```cmd
# Clone the repository
git lfs install
git clone <repository-url>
cd STEM_VCS

# Extract MinGW (if not already done)
# Extract tools/mingw.7z to tools/mingw64/

# Build
build_cli.bat
```

#### macOS/Linux
```bash
# Clone the repository
git clone <repository-url>
cd STEM_VCS

# Install dependencies
# macOS:
xcode-select --install

# Ubuntu/Debian:
sudo apt install build-essential

# Fedora:
sudo dnf install gcc make

# Build
make
```

### 2. Verify Build
```bash
# Windows
bin\stem.exe --version

# macOS/Linux
./bin/stem --version
```

---

## 📁 Project Structure

```
STEM_VCS/
├── src/
│   ├── core/          # Core VCS functionality
│   │   ├── init.c     # Repository initialization
│   │   ├── index.c    # Staging area management
│   │   ├── object.c   # Object storage
│   │   ├── reference.c# Branch/tag references
│   │   └── ...
│   └── cli/           # Command-line interface
│       ├── main.c     # Entry point
│       ├── cmd_*.c    # Individual commands
│       └── output.c   # Output formatting
├── include/
│   └── stem/          # Public headers
├── bin/               # Compiled binaries (generated)
├── obj/               # Object files (generated)
├── docs/              # Documentation
│   └── archive/       # Old documentation (historical)
├── tools/             # Build tools (MinGW for Windows)
├── Makefile           # Cross-platform build
├── CMakeLists.txt     # CMake build (alternative)
├── build_cli.bat      # Windows build script
└── install.sh         # Unix install script
```

---

## 🔨 Building from Source

### Using Make (Recommended)
```bash
make                # Build
make clean          # Clean build artifacts
make test           # Run basic tests
make install        # Install to system (Unix only)
```

### Using CMake
```bash
cmake -B build
cmake --build build
```

### Using Platform Scripts
```bash
# Windows
build_cli.bat

# Unix
./install.sh
```

---

## 🧪 Testing

### Manual Testing
```bash
# Create test repository
mkdir test_repo
cd test_repo
stem init

# Test basic operations
stem add .
stem commit -m "Test commit"
stem history

# Test branching
stem branch feature
stem switch feature
stem merge main
```

### Automated Testing
Currently manual testing only. Automated test suite contributions welcome!

---

## 📝 Coding Standards

### C Code Style

**1. Formatting**
- Use 4 spaces for indentation (no tabs)
- Opening braces on same line for functions and control structures
- Use snake_case for functions and variables
- Use UPPER_CASE for constants and macros

**Example:**
```c
int stem_init_repository(const char *path) {
    if (path == NULL) {
        return STEM_ERROR_INVALID_ARG;
    }
    
    // Implementation
    return STEM_SUCCESS;
}
```

**2. Error Handling**
- Always check return values
- Use error codes defined in `types.h`
- Clean up resources on error paths

**Example:**
```c
FILE *file = fopen(path, "r");
if (file == NULL) {
    fprintf(stderr, "Error: Cannot open %s\n", path);
    return STEM_ERROR_IO;
}

// Use file...

fclose(file);
return STEM_SUCCESS;
```

**3. Memory Management**
- Always free allocated memory
- Use `malloc/free`, not `new/delete`
- Check for NULL after allocation
- Avoid memory leaks

**Example:**
```c
char *buffer = malloc(1024);
if (buffer == NULL) {
    return STEM_ERROR_MEMORY;
}

// Use buffer...

free(buffer);
return STEM_SUCCESS;
```

**4. Compiler Warnings**
- Code must compile with **zero warnings**
- Use: `-Wall -Wextra -std=c11`
- Fix all warnings before submitting

**5. Comments**
- Use `//` for single-line comments
- Use `/* */` for multi-line comments
- Document non-obvious logic
- Include function header comments for public API

**Example:**
```c
/**
 * Initialize a new STEM repository at the specified path.
 * 
 * @param path Directory path for the repository
 * @return STEM_SUCCESS on success, error code on failure
 */
int stem_init_repository(const char *path);
```

### Cross-Platform Compatibility

**1. File Paths**
- Use forward slashes `/` in code where possible
- Use platform-specific separators only when required
- Test on Windows, macOS, and Linux

**2. Line Endings**
- Use `\n` for line endings in code
- Git will handle CRLF conversion automatically

**3. Platform-Specific Code**
```c
#ifdef _WIN32
    // Windows-specific code
#else
    // Unix-like systems (Linux, macOS)
#endif
```

---

## 🎯 Contribution Areas

### High Priority
- [ ] Unit test framework
- [ ] Integration tests
- [ ] Additional cloud providers (AWS S3, Azure, Dropbox)
- [ ] GUI application
- [ ] Performance optimizations
- [ ] Windows native PowerShell module

### Medium Priority
- [ ] Conflict resolution UI
- [ ] Interactive merge tools
- [ ] Diff visualization
- [ ] Git import/export
- [ ] Plugin system

### Documentation
- [ ] Video tutorials
- [ ] More examples
- [ ] Translation to other languages
- [ ] API documentation

### Bug Fixes
Always welcome! Check issues for known bugs.

---

## 📤 Submitting Changes

### Before Submitting
1. ✅ Build succeeds with zero warnings
2. ✅ Code follows style guidelines
3. ✅ Manual testing completed
4. ✅ No memory leaks (use valgrind on Linux)
5. ✅ Documentation updated if needed

### Pull Request Process
1. Fork the repository
2. Create a feature branch (`feature/my-feature`)
3. Make your changes
4. Test thoroughly on your platform
5. Commit with clear messages
6. Push to your fork
7. Open a pull request

### Commit Messages
```
Short summary (50 chars or less)

Longer explanation if needed. Explain:
- What changed
- Why it changed
- Any breaking changes
- Related issue numbers (#123)
```

**Examples:**
```
Add support for AWS S3 storage

Implements S3 bucket upload/download for stem push/pull.
Uses AWS SDK for C. Requires AWS credentials in config.

Fixes #42
```

---

## 🐛 Reporting Bugs

### Bug Report Template
```markdown
## Description
Brief description of the bug

## Steps to Reproduce
1. Run command X
2. Do action Y
3. Observe error Z

## Expected Behavior
What should happen

## Actual Behavior
What actually happens

## Environment
- OS: Windows 11 / macOS 13 / Ubuntu 22.04
- STEM Version: 1.0.0
- GCC Version: 13.2.0

## Additional Context
Error messages, logs, screenshots
```

### Where to Report
- GitHub Issues (if repository is on GitHub)
- Email: [maintainer email]
- Include full error output and `stem doctor` results

---

## 💡 Feature Requests

We welcome feature suggestions! Please include:
1. **Use case:** Why is this needed?
2. **Proposed solution:** How should it work?
3. **Alternatives:** Other approaches considered?
4. **Examples:** How would it be used?

---

## 🤝 Code of Conduct

### Our Standards
- Be respectful and inclusive
- Accept constructive criticism
- Focus on what's best for the project
- Show empathy towards others

### Unacceptable Behavior
- Harassment or discriminatory language
- Personal attacks
- Publishing others' private information
- Unprofessional conduct

---

## 📄 License

By contributing to STEM VCS, you agree that your contributions will be licensed under the MIT License.

---

## 🙏 Recognition

Contributors will be recognized in:
- CONTRIBUTORS.md file
- Release notes
- Project README

---

## 📞 Getting Help

- **Documentation:** README.md, MANUAL.md
- **Questions:** Open a discussion or issue
- **Chat:** [If available]

---

**Thank you for contributing to STEM VCS!**

Your efforts help make version control simpler and more accessible for everyone.
