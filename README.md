# phlego

This project aims to create a robust and flexible RISC-V emulator with advanced features and support for various extensions of the RISC-V ISA.

## Roadmap

### 1. **Multi-ISA Support**
- **Description:** Implement support for various RISC-V ISA extensions, including RV32IM, RV64I, RV32A, RV32F/D, etc.
- **Status:** Completed for RV32IM.

### 2. **Pipeline and Performance Modeling**
- **Description:** Simulate a 5-stage pipeline (Fetch, Decode, Execute, Memory, Write-back) with features like:
  - Branch prediction.
  - Data hazards detection and resolution.
  - Instruction-level parallelism.
  - Accurate stall cycles and forwarding.
- **Status:** 5-stage pipeline completed.

### 3. **Debugging and Visualization**
- **Description:**
  - Integrate an interactive debugger (e.g., GDB).
  - Provide GUI-based visualization for registers, memory, and pipeline stages.
  - Display pipeline activity and stall diagnostics.
- **Objective:** Enable real-time debugging and detailed simulation insights.

### 4. **Peripheral Support**
- **Description:** Add support for peripheral devices such as:
  - UART.
  - Timers.
  - Interrupt controllers.
- **Objective:** Create a complete system simulation environment.

### 5. **Cycle-Accurate Simulation**
- **Description:** Provide cycle-by-cycle simulation for accurate hardware behavior representation.
- **Enhancements:**
  - Synchronize instruction timing with pipeline stages.
  - Model bus communication and memory latencies.
  - Simulate contention for shared resources.
- **Objective:** Deliver precise hardware timing and execution.

### 6. **Performance Optimization**
- **Description:** Optimize the emulator for:
  - Handling large datasets.
  - Executing complex binaries efficiently.
- **Techniques:**
  - Leverage Just-In-Time (JIT) compilation for critical code paths.
  - Implement efficient memory access patterns.
- **Objective:** Achieve near-native execution speeds where possible.

### 7. **Multi-Threading and Parallelism**
- **Description:**
  - Implement support for multiple cores and threads.
  - Simulate parallel processing environments.
- **Objective:** Enable multi-core system simulations.

### 8. **Compatibility and Toolchain Integration**
- **Description:**
  - Ensure compatibility with RISC-V binaries and toolchains.
  - Integrate with external tools like profilers and tracing utilities.
  - Support ELF binary parsing with detailed symbol resolution.

### 9. **Test Suite and Validation**
- **Description:**
  - Incorporate official RISC-V test suites and benchmarks (e.g., SPEC CPU, CoreMark).
  - Develop unit tests for all instruction types.
  - Include performance regression tests.
- **Objective:** Validate emulator functionality and performance.

### 10. **Commercial-Grade Features**
- **Description:**
  - Add features found in commercial emulators, such as:
    - Advanced memory hierarchy (L1, L2 cache models).
    - Support for virtual memory and MMUs.
    - Power and thermal modeling.
    - Enhanced debugging tools with event tracing.
  - Provide an API for custom extensions and integrations.
- **Objective:** Compete with industry-standard solutions.

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