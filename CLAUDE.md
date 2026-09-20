# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Project Overview
Phlego is a RISC-V emulator project that implements a pipelined CPU architecture with support for loading and executing ELF files. The project follows a modular design with clear separation between CPU, memory, and runner components.

## Development Commands

### Building the Project
```bash
# Create and enter build directory
mkdir -p build && cd build

# Configure with CMake (default: Release with optimizations)
cmake ..

# Build the emulator
make

# The executable will be available at ./phlego
```

### Build Options
- `ENABLE_GDB_SERVER` (ON by default): Build with GDB server support
- `ENABLE_OPTIMIZATIONS` (ON by default): Enable -O3 optimizations for Release builds
- `ENABLE_TESTS` (ON by default): Build unit tests with GoogleTest

To customize options:
```bash
cmake -DENABLE_TESTS=OFF -DENABLE_OPTIMIZATIONS=OFF ..
```

### Running the Emulator
```bash
# Basic usage: provide path to ELF file
./phlego path/to/program.elf

# Example with built executable
./phlego test/riscv-tests/isa/rv32ui-p-add.dump
```

### Running Tests
```bash
# Build and run unit tests
ctest  # or ./test/unit/your_test_executable

# Run riscv-tests regression suite
EMULATOR=./build/phlego ./test/riscv_tests_regression.sh

# Run a single riscv-test
./build/phlego test/riscv-tests/isa/rv32ui-p-add.dump
```

### Code Formatting
The project uses clang-format. To format code:
```bash
# Format all source files
find src/ -name "*.cpp" -o -name "*.h" -o -name "*.hpp" | xargs clang-format -i

# Check formatting without applying
find src/ -name "*.cpp" -o -name "*.h" -o -name "*.hpp" | xargs clang-format --dry-run -Werror
```

## Code Architecture

### Core Components
1. **CPU** (`src/cpu.h`/`src/cpu.cpp`):
   - Implements RISC-V ISA with 5-stage pipeline (Fetch, Decode, Execute, Memory, WriteBack)
   - Thread-safe design with mutex protection for register access
   - Supports all base integer instructions (R-type, I-type, S-type, B-type, J-type, U-type)
   - Includes status register with halt and divide-by-zero flags

2. **Memory** (`src/memory.h`/`src/memory.cpp`):
   - Simple linear memory model with ELF loading capabilities
   - Provides stack pointer and initial address extraction from ELF files
   - 1MB default size (configurable in main.cpp)

3. **CPURunner** (`src/cpu_runner.h`/`src/cpu_runner.cpp`):
   - Orchestrates CPU execution cycle
   - Implements the main run loop that advances pipeline stages
   - Handles hazard detection and pipeline stalls (in progress based on branch name)

4. **Logger** (`src/logger.h`/`src/logger.cpp`):
   - Simple logging system with configurable levels (DEBUG, INFO, ERROR)
   - Used throughout the codebase for diagnostics

### Data Flow
```
main.cpp → Memory → CPU → CPURunner
  ↓              ↑     ↑
ELF File    ← Load  ← Instructions
```

### Pipeline Implementation
The CPU implements a classic 5-stage RISC-V pipeline:
1. **Fetch**: Reads instruction from memory at PC
2. **Decode**: Parses instruction into appropriate struct variant
3. **Execute**: Performs ALU operations or branch resolution
4. **Memory**: Handles load/store operations
5. **WriteBack**: Writes results to register file

Key features:
- Pipeline registers between each stage (FetchStage, DecodeStage, etc.)
- Thread-safe register access via mutex for external tools (GDB server)
- Hazard detection logic (in development based on branch name "hazard_stall2")
- Status flag handling for exceptional conditions

### Testing Structure
- **Unit Tests**: Located in `test/unit/` using GoogleTest framework
- **Regression Tests**: RISC-V compliance tests in `test/riscv-tests/`
- **Test Runner**: `test/riscv_tests_regression.sh` automates running all ISA tests

### Dependencies
- **ELFIO**: External project for ELF file parsing (automatically fetched by CMake)
- **GoogleTest**: External dependency for unit testing (automatically fetched when ENABLE_TESTS=ON)
- **Standard Library**: C++17 (required)

## Important Files to Understand
- `src/main.cpp`: Entry point showing typical usage pattern
- `src/cpu.h`: Defines instruction structs, pipeline stages, and CPU class interface
- `src/cpu.cpp`: Implements CPU logic including pipeline stage execution
- `src/cpu_runner.h/.cpp`: Controls execution cycle
- `src/memory.h/.cpp`: Memory subsystem with ELF loading

## Code Conventions
- Uses C++17 features
- Instruction decoding uses `std::variant` for type-safe instruction handling
- Pipeline stages represented as structs with validity flags
- Register access is thread-safe via `std::mutex`
- Error handling through exceptions and status flags
- Logging levels configurable via LOG_LEVEL macro

## Common Tasks
### Adding New Instructions
1. Add opcode/funct3/funct7 enums in `cpu.h`
2. Define instruction struct if needed
3. Add decode helper method in CPU class
4. Add execute helper method in CPU class
5. Update decode() and execute() methods to handle new instruction type
6. Add corresponding execute_*_type() method if needed

### Modifying Pipeline Behavior
1. Pipeline stage structs are in `cpu.h`
2. Stage execution methods are in `cpu.cpp` (fetch, decode, execute, mem, write_back)
3. Hazard detection logic would be added in the runner or CPU stage methods