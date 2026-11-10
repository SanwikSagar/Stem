# Contributing to STEM

Thank you for your interest in contributing to STEM! This document provides guidelines for contributing to the project.

## Code of Conduct

Be respectful, professional, and constructive in all interactions.

## How to Contribute

### Reporting Bugs

1. Check if the bug has already been reported in Issues
2. Create a new issue with:
   - Clear title and description
   - Steps to reproduce
   - Expected vs actual behavior
   - System information (OS, compiler version)
   - Relevant logs or error messages

### Suggesting Features

1. Check if the feature has been suggested
2. Create an issue describing:
   - The problem it solves
   - Proposed solution
   - Alternative solutions considered
   - Implementation ideas

### Code Contributions

1. **Fork the repository**
2. **Create a branch**: `git checkout -b feature/my-feature`
3. **Write code following our style guide**
4. **Add tests** for new functionality
5. **Update documentation** as needed
6. **Commit with clear messages**: `Add lazy object fetching`
7. **Push to your fork**: `git push origin feature/my-feature`
8. **Create a Pull Request**

## Development Setup

```bash
# Clone repository
git clone https://github.com/yourusername/stem-vcs
cd stem-vcs

# Setup LVGL
./setup_lvgl.sh  # or setup_lvgl.bat on Windows

# Install dependencies
# Windows: vcpkg install sdl2 openssl zlib
# macOS: brew install sdl2 openssl zlib
# Linux: sudo apt install libsdl2-dev libssl-dev zlib1g-dev

# Build
mkdir build && cd build
cmake .. -DBUILD_GUI=ON -DBUILD_CLI=ON -DBUILD_TESTS=ON
cmake --build .

# Run tests
ctest
```

## Coding Style

### C Code Style

- **Standard**: C99
- **Indentation**: 4 spaces (no tabs)
- **Braces**: K&R style
- **Naming**:
  - Functions: `snake_case`
  - Types: `snake_case_t`
  - Constants: `UPPER_CASE`
  - Macros: `UPPER_CASE`

```c
/* Good example */
stem_error_t stem_object_write(stem_repository_t *repo, 
                                 const stem_object_header_t *obj,
                                 stem_hash_t *hash_out) {
    if (!repo || !obj || !hash_out) {
        return STEM_ERROR_INVALID_ARG;
    }
    
    /* Implementation */
    return STEM_OK;
}
```

### Comments

- Use `/* */` for multi-line comments
- Use `//` for single-line comments
- Document all public API functions
- Explain WHY, not WHAT (code shows what)

### Error Handling

- Always check return values
- Use `stem_error_t` for all fallible operations
- Set `stem_last_error()` for detailed errors
- Never use `assert()` for input validation

## Testing

- Write unit tests for new functions
- Test edge cases and error conditions
- Ensure tests pass before submitting PR
- Add integration tests for new features

## Documentation

- Update README.md for user-facing changes
- Update API documentation in headers
- Add examples for new features
- Keep documentation concise and clear

## Commit Messages

Format:
```
<type>: <subject>

<body>

<footer>
```

Types:
- `feat`: New feature
- `fix`: Bug fix
- `docs`: Documentation
- `style`: Formatting
- `refactor`: Code restructuring
- `test`: Tests
- `chore`: Build/tooling

Example:
```
feat: Add lazy object fetching

Implements on-demand object download from cloud storage.
Objects are fetched only when accessed, enabling instant clones
of large repositories.

Closes #42
```

## Pull Request Process

1. **Update TASK_PROGRESS.md** with completed items
2. **Add tests** and ensure all pass
3. **Update documentation**
4. **Keep PR focused** - one feature per PR
5. **Respond to feedback** promptly
6. **Rebase if needed** to keep history clean

## Areas We Need Help With

- 🔐 **Encryption**: ChaCha20-Poly1305, AES-GCM implementation
- ☁️ **Cloud Providers**: Google Drive, Dropbox, OneDrive OAuth & API
- 🎨 **GUI**: LVGL components, file tree, diff viewer
- 📦 **Binary Optimization**: Chunking, deduplication, compression
- 🧪 **Testing**: Unit tests, integration tests, fuzzing
- 📝 **Documentation**: Tutorials, examples, API docs
- 🌐 **Localization**: Translations

## Questions?

Create an issue with the `question` label or reach out to maintainers.

Thank you for contributing to STEM! 🌱
