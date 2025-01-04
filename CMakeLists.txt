cmake_minimum_required(VERSION 3.10)

# Project name and version
project(Phlego VERSION 1.0)

# Set the C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# Set the logging level (DEBUG, INFO, ERROR)
if(NOT DEFINED LOG_LEVEL)
    set(LOG_LEVEL LOG_LEVEL_INFO)
endif()
add_compile_definitions(LOG_LEVEL=${LOG_LEVEL})

# Include directories
include_directories(${CMAKE_SOURCE_DIR}/src)

# Source files
set(SOURCES
    src/cpu.cpp
    src/memory.cpp
    src/main.cpp
)

# External project for ELFIO
include(ExternalProject)
ExternalProject_Add(ELFIO
    GIT_REPOSITORY https://github.com/serge1/ELFIO.git
    GIT_TAG Release_3.12
    PREFIX ${CMAKE_BINARY_DIR}/elfio
    CONFIGURE_COMMAND ""
    BUILD_COMMAND ""
    INSTALL_COMMAND ""
    LOG_DOWNLOAD ON
    LOG_UPDATE ON
    LOG_CONFIGURE ON
    LOG_BUILD ON
    LOG_INSTALL ON
)

# Include ELFIO headers
ExternalProject_Get_Property(ELFIO source_dir)
include_directories(${source_dir})

# Executable
add_executable(phlego ${SOURCES})

# Ensure ELFIO is downloaded before building the emulator
add_dependencies(phlego ELFIO)

# Compiler warnings
target_compile_options(phlego PRIVATE -Wall -Wextra)