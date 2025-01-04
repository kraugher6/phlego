# phlego

This project aims to create a robust and flexible RISC-V emulator with advanced features and support for various extensions of the RISC-V ISA.

## Roadmap

### 1. **Support Multiple ISAs**
   - [x] Implement RV32IM.
   - [ ] Add support for other extensions like RV64I, RV32A, RV32F/D, etc., with dynamic selection without recompilation.

### 2. **Pipeline and Performance Modeling**
   - [ ] Simulate the behavior of pipelines and branch prediction to enable performance analysis.

### 3. **Debugging and Visualization**
   - [ ] Integrate with debuggers like GDB and provide a GUI for interactive debugging of registers, memory, and pipeline stages.

### 4. **Peripheral Support**
   - [ ] Add support for devices like UART, timers, and interrupt controllers.

### 5. **Cycle-Accurate Simulation**
   - [ ] Provide a simulation model with cycle-accurate precision.

### 6. **Performance Optimization**
   - [ ] Optimize the emulator to handle large datasets and complex binaries efficiently.

### 7. **Multithreading and Parallelism**
   - [ ] Implement support for multiple CPUs or parallel cores.

### 8. **Compatibility and Toolchain Integration**
   - [ ] Ensure compatibility with standard binaries and toolchains, integrating debugging, profiling, and tracing tools.

### 9. **Test Suite and Validation**
   - [ ] Integrate official RISC-V test suites and benchmarks like SPEC CPU and CoreMark to validate and test the emulator.

## Current Progress

- Implemented RV32I.
- Implemented RV32M
- Next steps:
  1. Expand ISA support.
  2. Start validation and testing with official RISC-V test suites (Point 9).

## Features

- Modular design with extensible architecture.
- Support for dynamic ISA selection (ongoing).
- Plans for advanced debugging and visualization.

## Getting Started

### Prerequisites

- CMake 3.10 or higher
- g++17 compiler
- Git

### Dependency

- ELFIO library (https://github.com/serge1/ELFIO.git)
- riscv-gnu-toolchain (https://github.com/riscv-collab/riscv-gnu-toolchain) to compile the tests

### Building the Project

To build the project using CMake, follow these steps:

1. Create a build directory:
    ```sh
    mkdir build
    cd build
    ```

2. Run CMake to configure the project:
    ```sh
    cmake ..
    ```

3. Build the project:
    ```sh
    make
    ```

### Running Tests

To run tests, navigate to the appropriate test directory and use the provided Makefile. For example:

```sh
cd tests/rv32m
make
../../build/phlego rv32m.bin
```