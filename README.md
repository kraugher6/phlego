# Phlego

[![License: MIT](https://img.shields.io/badge/License-MIT-yellow.svg)](https://opensource.org/licenses/MIT)
[![C++17](https://img.shields.io/badge/C%2B%2B-17-blue.svg)](https://en.wikipedia.org/wiki/C%2B%2B17)
[![Build Status](https://img.shields.io/badge/build-passing-brightgreen)](https://github.com/yourusername/phlego/actions)

A robust and flexible RISC-V emulator designed for learning computer architecture concepts. Phlego implements a pipelined CPU architecture with support for loading and executing ELF files, making it ideal for educational purposes and architecture experimentation.

## Overview

Phlego simulates a 5-stage RISC-V pipeline (Fetch, Decode, Execute, Memory, Write-back) with complete hazard detection and forwarding capabilities. The emulator supports the RV32IM base integer instruction set and can execute RISC-V ELF binaries.

## Features

- 🔧 **5-Stage Pipelined Architecture**: Fetch, Decode, Execute, Memory, Write-back stages
- ⚡ **Hazard Detection and Forwarding**: Implements data hazard detection and resolution
- 📋 **RV32IM Instruction Set Support**: Complete base integer instruction set (R-type, I-type, S-type, B-type, J-type, U-type)
- 📂 **ELF File Loading**: Built-in ELF parser using ELFIO library
- 🐛 **GDB Server Support**: Optional GDB server for debugging (enabled by default)
- 📊 **Configurable Logging**: Adjustable log levels (DEBUG, INFO, ERROR)
- 🧪 **Comprehensive Testing**: Unit tests with GoogleTest and RISC-V compliance test suite
- ⚙️ **Modular Design**: Clean separation of CPU, Memory, and Runner components

## Roadmap

### Completed Features
- ✅ **RV32IM Base Integer Instruction Set**
- ✅ **5-Stage Pipeline Implementation**
- ✅ **Complete Hazard Detection and Forwarding**
- ✅ **ELF File Loading**
- ✅ **Thread-Safe Register Access**

### In Progress
- 🔄 **Branch Prediction Implementation**
- 🔄 **Performance Counters and Statistics**

### Planned Features
- 📝 **Interrupt and Exception Handling**
- 📝 **Memory Management Unit (MMU) Support**
- 📝 **Peripheral Devices (UART, Timers)**
- 📝 **Cycle-Accurate Timing Modeling**
- 📝 **Just-In-Time (JIT) Compilation for Performance**
- 📝 **Multi-Core/SMP Support**
- 📝 **GUI-Based Debugger and Visualizer**
- 📝 **Advanced Memory Hierarchy (Cache Models)**
- 📝 **Power and Thermal Modeling**

## Getting Started

### Prerequisites

- CMake 3.14 or higher
- GCC 9+ or Clang 7+ with C++17 support
- Git
- Python 3.x (for test scripts)

### Optional Dependencies
- riscv-gnu-toolchain (for compiling RISC-V test programs)
- GDB (for debugging with the built-in GDB server)

### Building the Project

```bash
# Clone the repository
git clone https://github.com/yourusername/phlego.git
cd phlego

# Create and enter build directory
mkdir -p build && cd build

# Configure with CMake (Release build with optimizations)
cmake ..

# Build the emulator
make -j$(nproc)

# The executable will be available at ./phlego
```

### Build Options

Customize the build with these CMake options:

| Option | Default | Description |
|--------|---------|-------------|
| `ENABLE_GDB_SERVER` | ON | Build with GDB server support |
| `ENABLE_OPTIMIZATIONS` | ON | Enable -O3 optimizations for Release builds |
| `ENABLE_TESTS` | ON | Build unit tests with GoogleTest |

Example with custom options:
```bash
cmake -DENABLE_TESTS=OFF -DENABLE_OPTIMIZATIONS=OFF ..
```

### Running the Emulator

```bash
# Basic usage: provide path to RISC-V ELF file
./phlego path/to/program.elf

# Example with a simple test program
./phlego test/riscv-tests/isa/rv32ui-p-add.dump

# Run with verbose logging
LOG_LEVEL=LOG_LEVEL_INFO ./phlego test/riscv-tests/isa/rv32ui-p-add.dump
```

### Running Tests

```bash
# Build and run unit tests
ctest  # or ./test/unit/*_test

# Run riscv-tests regression suite
EMULATOR=./build/phlego ./test/riscv_tests_regression.sh

# Run a single riscv-test
./build/phlego test/riscv-tests/isa/rv32ui-p-add.dump

# Run unit tests with verbose output
ctest --output-on-failure
```

## Project Structure

```
phlego/
├── src/                 # Source code
│   ├── cpu.h/cpu.cpp    # CPU implementation with pipeline
│   ├── memory.h/memory.cpp # Memory subsystem with ELF loading
│   ├── cpu_runner.h/cpu_runner.cpp # Execution orchestrator
│   ├── logger.h/logger.cpp # Configurable logging system
│   ├── gdb_server.h/gdb_server.cpp # Optional GDB server
│   └── main.cpp         # Entry point
├── test/                # Test directories
│   ├── unit/            # GoogleTest unit tests
│   └── riscv-tests/     # RISC-V compliance test suite
├── build/               # Build directory (created during build)
├── script/              # Helper scripts
│   └── lint/            # Linting configuration and logs
├── .clang-format        # Code formatting configuration
├── .clang-tidy          # Static analysis configuration
├── CMakeLists.txt       # Build configuration
└── CLAUDE.md            # AI assistant guidance
```

## Code Conventions

- **Language**: C++17
- **Naming**:
  - Classes: `PascalCase` (e.g., `CPU`, `Memory`)
  - Functions/Methods: `camelCase` (e.g., `fetch()`, `executeInstruction()`)
  - Variables: `camelCase` (e.g., `programCounter`, `registerFile`)
  - Constants: `UPPER_SNAKE_CASE` (e.g., `MAX_MEMORY_SIZE`, `DEFAULT_LOG_LEVEL`)
- **Comments**: English only, explain *why* not *what*
- **Error Handling**: Exceptions for unrecoverable errors, status flags for CPU-specific conditions
- **Thread Safety**: Mutex protection for shared state (registers, pipeline)

## Pipeline Implementation Details

Phlego implements a classic 5-stage RISC-V pipeline with the following characteristics:

1. **Fetch Stage**: Reads instruction from memory at PC address
2. **Decode Stage**: Parses 32-bit instruction into typed struct variants using `std::variant`
3. **Execute Stage**: Performs ALU operations, branch target calculation
4. **Memory Stage**: Handles load/store operations to data memory
5. **Write-back Stage**: Writes results to register file

### Key Features
- **Pipeline Registers**: Dedicated structs between each stage with validity flags
- **Forwarding Unit**: Detects and forwards results from later stages to earlier stages
- **Hazard Detection**: Identifies data hazards that require pipeline stalls
- **Thread-Safe Register Access**: External tools (like GDB server) can safely read registers
- **Status Register**: Tracks halt condition, divide-by-zero, and other exceptions

## Extending the Emulator

### Adding New Instructions

1. Add opcode/funct3/funct7 values to the appropriate enums in `cpu.h`
2. Define instruction struct if needed (or use existing I-Type/R-Type/etc.)
3. Add decode helper method in `CPU` class
4. Add execute helper method in `CPU` class
5. Update `decode()` and `execute()` methods in `cpu.cpp` to handle new instruction
6. Add corresponding `execute_*_type()` method if specialized handling is needed

### Modifying Pipeline Behavior

1. Pipeline stage structures are defined in `cpu.h`
2. Stage execution methods are in `cpu.cpp`: `fetch()`, `decode()`, `execute()`, `mem()`, `write_back()`
3. Hazard detection logic can be added in the stage methods or in `CPURunner`
4. Performance counters can be added by extending the `CPU` class with statistics fields

## Contributing

Please read [CONTRIBUTING.md](CONTRIBUTING.md) for details on our code of conduct and the process for submitting pull requests.

## License

This project is licensed under the MIT License - see the [LICENSE](LICENSE) file for details.

## Acknowledgments

- ELFIO library for ELF file parsing
- GoogleTest for unit testing framework
- RISC-V International for the open ISA specification
- The riscv-tests project for compliance test suites

## Contact

For questions and support, please open an issue on the GitHub repository.