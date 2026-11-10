cmake_minimum_required(VERSION 3.10)
project(STEM_VCS VERSION 1.0.0 LANGUAGES C)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)

# Build options
option(BUILD_CLI "Build CLI tool" ON)
option(BUILD_GUI_WEB "Build Web GUI server" ON)

# Platform detection
if(WIN32)
    add_definitions(-D_CRT_SECURE_NO_WARNINGS)
    add_definitions(-DWIN32_LEAN_AND_MEAN)
endif()

# Include directories
include_directories(${CMAKE_SOURCE_DIR}/include)

# Core library sources
file(GLOB CORE_SOURCES "src/core/*.c")

# Create core library
add_library(stem_core STATIC ${CORE_SOURCES})

# CLI executable
if(BUILD_CLI)
    file(GLOB CLI_SOURCES "src/cli/*.c")
    add_executable(stem ${CLI_SOURCES})
    target_link_libraries(stem stem_core)
    
    # Platform-specific libraries
    if(WIN32)
        target_link_libraries(stem ws2_32)
    endif()
    
    # Compiler flags
    target_compile_options(stem PRIVATE -O2 -Wall -Wextra)
    target_compile_options(stem_core PRIVATE -O2 -Wall -Wextra)
    
    # Installation
    install(TARGETS stem DESTINATION bin)
endif()

# GUI executable (future)
if(BUILD_GUI)
    message(WARNING "GUI is not yet implemented")
    # file(GLOB GUI_SOURCES "src/gui/*.c")
    # add_executable(stem_gui ${GUI_SOURCES})
    # target_link_libraries(stem_gui stem_core)
endif()

# Web GUI server
if(BUILD_GUI_WEB)
    add_executable(stem_gui src/gui_web/server.c)
    target_link_libraries(stem_gui stem_core)
    
    # Platform-specific libraries
    if(WIN32)
        target_link_libraries(stem_gui ws2_32)
    endif()
    
    # Compiler flags
    target_compile_options(stem_gui PRIVATE -O2 -Wall -Wextra)
    
    # Installation
    install(TARGETS stem_gui DESTINATION bin)
endif()
