cmake_minimum_required(VERSION 3.14)

# Project name and version
project(Phlego VERSION 1.0 LANGUAGES CXX)

# Set the C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# Set Output Directories
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)

# Build options
option(ENABLE_TESTS "Enable building unit tests" ON)

# Source files
set(CORE_SOURCES
    src/core/decoder.cpp
    src/core/interpreter.cpp
    src/core/register_file.cpp
    src/core/memory.cpp
    src/core/pipeline.cpp
    src/core/runner.cpp
    src/hazard/hazard_unit.cpp
    src/hazard/forwarding_unit.cpp
    src/prediction/branch_predictor.cpp
    src/cache/cache.cpp
    src/ooo/scoreboard.cpp
    src/ooo/tomasulo.cpp
    src/utils/elf_loader.cpp
    src/logger.cpp
)

# Executable-only sources
set(EXEC_SOURCES
    src/main.cpp
)

# Build core library and executable
add_library(phlego_core STATIC ${CORE_SOURCES})
add_executable(phlego ${EXEC_SOURCES})

# Include project headers
target_include_directories(phlego_core PUBLIC ${CMAKE_SOURCE_DIR}/src)
target_include_directories(phlego PRIVATE ${CMAKE_SOURCE_DIR}/src)

# Link core into the executable
target_link_libraries(phlego PRIVATE phlego_core)

# Compiler warnings and options
if(MSVC)
    target_compile_options(phlego_core PRIVATE /W4 /WX)
    target_compile_options(phlego PRIVATE /W4 /WX)
else()
    target_compile_options(phlego_core PRIVATE -Wall -Wextra -Wpedantic -Wshadow -Wno-unused-parameter)
    target_compile_options(phlego PRIVATE -Wall -Wextra -Wpedantic -Wshadow -Wno-unused-parameter)
endif()

# Build specific flags
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -O3")
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -g -O0")

# Install target
install(TARGETS phlego DESTINATION bin)

# Unit tests (GoogleTest)
if(ENABLE_TESTS)
    include(FetchContent)
    FetchContent_Declare(
        googletest
        URL https://github.com/google/googletest/archive/refs/heads/main.zip
        DOWNLOAD_EXTRACT_TIMESTAMP TRUE
    )
    FetchContent_MakeAvailable(googletest)

    enable_testing()
    add_subdirectory(test/unit)
endif()