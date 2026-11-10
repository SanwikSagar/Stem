# STEM VCS - Cross-Platform Makefile
# Supports: Linux, macOS, Windows (with MinGW/MSYS2)

# Compiler
CC = gcc

# Directories
SRC_DIR = src
CORE_DIR = $(SRC_DIR)/core
CLI_DIR = $(SRC_DIR)/cli
INC_DIR = include
OBJ_DIR = obj
BIN_DIR = bin

# Output
TARGET = $(BIN_DIR)/stem

# Compiler flags
CFLAGS = -O2 -Wall -Wextra -I$(INC_DIR) -std=c11
LDFLAGS = 

# Platform detection
ifeq ($(OS),Windows_NT)
    TARGET := $(TARGET).exe
    LDFLAGS += -lws2_32
    MKDIR = if not exist "$(1)" mkdir "$(1)"
    RM = del /Q
    RMDIR = rmdir /S /Q
else
    UNAME_S := $(shell uname -s)
    ifeq ($(UNAME_S),Linux)
        CFLAGS += -D_GNU_SOURCE
    endif
    ifeq ($(UNAME_S),Darwin)
        CFLAGS += -D_DARWIN_C_SOURCE
    endif
    MKDIR = mkdir -p $(1)
    RM = rm -f
    RMDIR = rm -rf
endif

# Source files
CORE_SRCS = $(wildcard $(CORE_DIR)/*.c)
CLI_SRCS = $(wildcard $(CLI_DIR)/*.c)
ALL_SRCS = $(CORE_SRCS) $(CLI_SRCS)

# Object files
CORE_OBJS = $(patsubst $(CORE_DIR)/%.c,$(OBJ_DIR)/%.o,$(CORE_SRCS))
CLI_OBJS = $(patsubst $(CLI_DIR)/%.c,$(OBJ_DIR)/%.o,$(CLI_SRCS))
ALL_OBJS = $(CORE_OBJS) $(CLI_OBJS)

# Default target
.PHONY: all
all: banner $(TARGET)

# Banner
.PHONY: banner
banner:
	@echo "========================================"
	@echo "  STEM VCS - Cross-Platform Build"
	@echo "========================================"
	@echo ""

# Create directories
$(OBJ_DIR):
	@$(call MKDIR,$(OBJ_DIR))

$(BIN_DIR):
	@$(call MKDIR,$(BIN_DIR))

# Compile core objects
$(OBJ_DIR)/%.o: $(CORE_DIR)/%.c | $(OBJ_DIR)
	@echo "[CC] $<"
	@$(CC) $(CFLAGS) -c $< -o $@

# Compile CLI objects
$(OBJ_DIR)/%.o: $(CLI_DIR)/%.c | $(OBJ_DIR)
	@echo "[CC] $<"
	@$(CC) $(CFLAGS) -c $< -o $@

# Link executable
$(TARGET): $(ALL_OBJS) | $(BIN_DIR)
	@echo ""
	@echo "[LINK] Creating executable..."
	@$(CC) $(ALL_OBJS) -o $(TARGET) $(LDFLAGS)
	@echo ""
	@echo "========================================"
	@echo "  BUILD SUCCESSFUL!"
	@echo "========================================"
	@echo ""
	@echo "Created: $(TARGET)"
	@echo ""

# Clean build artifacts
.PHONY: clean
clean:
	@echo "Cleaning build artifacts..."
	@$(RMDIR) $(OBJ_DIR) 2>/dev/null || true
	@$(RMDIR) $(BIN_DIR) 2>/dev/null || true
	@echo "Clean complete."

# Install to system (Unix-like systems)
.PHONY: install
install: $(TARGET)
	@echo "Installing STEM VCS..."
ifeq ($(OS),Windows_NT)
	@echo "On Windows, run INSTALL.bat instead"
else
	@install -d $(DESTDIR)/usr/local/bin
	@install -m 755 $(TARGET) $(DESTDIR)/usr/local/bin/stem
	@echo "Installed to /usr/local/bin/stem"
	@echo "Run 'stem --version' to verify"
endif

# Uninstall
.PHONY: uninstall
uninstall:
	@echo "Uninstalling STEM VCS..."
ifeq ($(OS),Windows_NT)
	@echo "On Windows, manually remove from PATH"
else
	@$(RM) $(DESTDIR)/usr/local/bin/stem
	@echo "Uninstalled from /usr/local/bin/stem"
endif

# Test build
.PHONY: test
test: $(TARGET)
	@echo ""
	@echo "Running tests..."
	@$(TARGET) --version
	@echo ""
	@echo "Build test passed!"

# Help
.PHONY: help
help:
	@echo "STEM VCS Build System"
	@echo ""
	@echo "Targets:"
	@echo "  make          - Build STEM VCS"
	@echo "  make clean    - Remove build artifacts"
	@echo "  make install  - Install to system (Unix)"
	@echo "  make uninstall- Remove from system (Unix)"
	@echo "  make test     - Test build"
	@echo "  make help     - Show this help"
	@echo ""
	@echo "Platform: $(shell uname -s 2>/dev/null || echo Windows)"
	@echo "Compiler: $(CC)"

.PHONY: info
info:
	@echo "Build Information:"
	@echo "  Platform:  $(shell uname -s 2>/dev/null || echo Windows)"
	@echo "  Compiler:  $(CC)"
	@echo "  CFLAGS:    $(CFLAGS)"
	@echo "  LDFLAGS:   $(LDFLAGS)"
	@echo "  Target:    $(TARGET)"
	@echo "  Sources:   $(words $(ALL_SRCS)) files"
