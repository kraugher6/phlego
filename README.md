# phlego

This project aims to create a robust and flexible RISC-V emulator with advanced features and support for various extensions of the RISC-V ISA.

## Roadmap

### 1. **Instruction Set Support**
   - [x] Support for RV32I base integer instruction set.
   - [x] Support for RV32M multiplication and division extension.
   - [ ] Support for RV32F floating-point extension.

### 2. **Pipeline Implementation**
   - [x] Five-stage pipeline: Fetch, Decode, Execute, Memory, Write-back.
   - [ ] Hazard detection and forwarding mechanisms.

### 3. **Memory Management**
   - [ ] Support for loading and storing data.
   - [ ] Memory-mapped I/O support.

### 4. **Debugging and Profiling**
   - [ ] Integrated debugging tools with breakpoints and step execution.
   - [ ] Performance profiling and logging.

### 5. **Simulation and Accuracy**
   - [ ] Cycle-accurate simulation model.
   - [ ] Accurate exception and interrupt handling.

### 6. **Performance Optimization**
   - [ ] Optimized for handling large datasets and complex binaries efficiently.

### 7. **Multithreading and Parallelism**
   - [ ] Support for multiple CPUs or parallel cores.

### 8. **Compatibility and Toolchain Integration**
   - [x] Compatibility with standard RISC-V binaries and toolchains.
   - [ ] Integration with debugging, profiling, and tracing tools.

### 9. **Test Suite and Validation**
   - [ ] Integration with official RISC-V test suites and benchmarks like SPEC CPU and CoreMark.

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