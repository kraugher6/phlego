# Contributing to Phlego

Thank you for your interest in contributing to Phlego! This document provides guidelines and instructions for contributing to this RISC-V emulator project.

## Table of Contents
- [Code of Conduct](#code-of-conduct)
- [How Can I Contribute?](#how-can-i-contribute)
  - [Reporting Bugs](#reporting-bugs)
  - [Suggesting Features](#suggesting-features)
  - [Your First Code Contribution](#your-first-code-contribution)
  - [Pull Requests](#pull-requests)
- [Development Setup](#development-setup)
- [Code Style Guidelines](#code-style-guidelines)
- [Testing Guidelines](#testing-guidelines)
- [Documentation Guidelines](#documentation-guidelines)
- [Community](#community)

## Code of Conduct

Please note that this project is released with a Contributor Code of Conduct. By participating in this project you agree to abide by its terms. Please ensure your interactions are respectful and professional.

## How Can I Contribute?

### Reporting Bugs

Before submitting a bug report, please check if the issue has already been reported by searching the [issue tracker](https://github.com/yourusername/phlego/issues).

When you are ready to report a bug, please include:
- A clear and descriptive title
- Steps to reproduce the issue
- Expected behavior vs. actual behavior
- Any relevant logs or error messages
- Information about your environment (OS, compiler version, etc.)
- Minimal test case if possible

### Suggesting Features

Feature requests are welcome! Please open an issue describing:
- The problem your feature would solve
- How the feature would work
- Any potential drawbacks or considerations
- Examples of how the feature would be used

### Your First Code Contribution

Looking for a place to start? Check out issues labeled with:
- `good first issue` - Issues suitable for newcomers
- `help wanted` - Issues that need attention
- `documentation` - Documentation improvements

### Pull Requests

1. Fork the repository and create your branch from `main`
2. If you've added code that should be tested, add tests
3. If you've changed APIs, update the documentation
4. Ensure the test suite passes
5. Make sure your code follows the project's code style
6. Issue your pull request!

#### Pull Request Guidelines
- Keep changes focused; if adding multiple features, consider multiple PRs
- Write clear, descriptive commit messages
- Reference related issues in your PR description (e.g., "Fixes #123")
- Include tests for new functionality
- Update documentation as needed
- Follow the code style guidelines

## Development Setup

### Prerequisites
- CMake 3.14 or higher
- GCC 9+ or Clang 7+ with C++17 support
- Git
- Python 3.x (for test scripts)

### Optional Dependencies
- riscv-gnu-toolchain (for compiling RISC-V test programs)
- GDB (for debugging with the built-in GDB server)
- clang-format and clang-tidy (for code quality)

### Building for Development

```bash
# Clone your fork
git clone https://github.com/yourusername/phlego.git
cd phlego

# Create development branch
git checkout -b feature/your-feature-name

# Create and enter build directory
mkdir -p build && cd build

# Configure with CMake (Debug build for development)
cmake -DCMAKE_BUILD_TYPE=Debug -DENABLE_TESTS=ON ..

# Build the emulator and tests
make -j$(nproc)
```

### Running Tests During Development

```bash
# Build and run unit tests
ctest  # or ./test/unit/*_test

# Run riscv-tests regression suite
EMULATOR=../build/phlego ./test/riscv_tests_regression.sh

# Run a single riscv-test
../build/phlego test/riscv-tests/isa/rv32ui-p-add.dump

# Run unit tests with verbose output
ctest --output-on-failure
```

### Code Formatting

The project uses clang-format. Please format your code before committing:

```bash
# Format all source files
find ../src/ -name "*.cpp" -o -name "*.h" -o -name "*.hpp" | xargs clang-format -i

# Check formatting without applying
find ../src/ -name "*.cpp" -o -name "*.h" -o -name "*.hpp" | xargs clang-format --dry-run -Werror
```

### Static Analysis

Run clang-tidy to catch potential issues:

```bash
# Run clang-tidy on source files
find ../src/ -name "*.cpp" -o -name "*.h" -o -name "*.hpp" | xargs clang-tidy -p ../build --
```

## Code Style Guidelines

### Language Standard
- C++17
- Avoid C++20 features unless absolutely necessary and approved

### Naming Conventions
- **Classes**: `PascalCase` (e.g., `CPU`, `Memory`)
- **Functions/Methods**: `camelCase` (e.g., `fetch()`, `executeInstruction()`)
- **Variables**: `camelCase` (e.g., `programCounter`, `registerFile`)
- **Constants**: `UPPER_SNAKE_CASE` (e.g., `MAX_MEMORY_SIZE`, `DEFAULT_LOG_LEVEL`)
- **Enumerations**: `PascalCase` for enum name, `UPPER_SNAKE_CASE` for values
- **Files**: `snake_case` (e.g., `cpu_runner.cpp`, `memory.h`)

### Commenting Style
- Use English only for all comments
- Explain **why** something is done, not **what** is done
- Use Doxygen-style comments for public APIs:
  ```cpp
  /**
   * Brief description of what the function does.
   *
   * @param param_name Description of parameter.
   * @return Description of return value.
   */
  ```
- Avoid obvious comments like "// increment by 1"
- Never use Italian comments or variable names

### Code Formatting
- Follow the `.clang-format` configuration (LLVM-based)
- Maximum line length: 100 characters
- Indentation: 2 spaces
- Pointer/reference alignment: align with type (e.g., `int* ptr`)

### Header Files
- Use include guards or `#pragma once`
- Minimize includes in headers; use forward declarations when possible
- Order of includes:
  1. C library headers
  2. C++ library headers
  3. Other libraries' headers
  4. Project's headers

### Exception Handling
- Use exceptions for unrecoverable errors
- Use CPU status flags for recoverable CPU-specific conditions
- Provide meaningful error messages
- Catch exceptions at appropriate boundaries

### Thread Safety
- The CPU class uses a mutex for thread-safe register access
- When adding new shared state, consider thread safety implications
- Document thread safety guarantees in class comments

## Testing Guidelines

### Test Philosophy
- Write unit tests for new functionality
- Test both normal operation and edge cases
- Tests should be fast, isolated, and repeatable
- Follow the Arrange-Act-Assert pattern

### Unit Tests
- Located in `test/unit/`
- Use GoogleTest framework
- Test naming: `[UnitUnderTest]_[Scenario]_[ExpectedResult]`
- Example: `CPU_Addition_WithOverflow_SetsCarryFlag`

### Regression Tests
- Located in `test/riscv-tests/`
- Official RISC-V compliance test suite
- Run using `./test/riscv_tests_regression.sh`

### Test Coverage
- Aim for high coverage of new code
- Focus on testing complex logic and edge cases
- Simple getters/setters may not need explicit tests if covered by other tests

## Documentation Guidelines

### In-Code Documentation
- Public classes and methods must have Doxygen-style comments
- Complex algorithms should have explanatory comments
- Document thread safety, exception guarantees, and performance characteristics
- Keep comments up-to-date when modifying code

### File-Level Documentation
- Each major component should have a brief description at the top of the file
- Example:
  ```cpp
  /**
   * @file cpu.h
   * @brief CPU implementation with 5-stage pipeline
   * @details Contains the CPU class implementing the RISC-V ISA
   *          with pipeline stages: Fetch, Decode, Execute, Memory, WriteBack
   */
  ```

### External Documentation
- Update README.md for user-facing changes
- Update CLAUDE.md for AI assistant guidance
- Add examples to documentation when adding significant features
- Keep documentation in sync with code changes

## Community

### Communication
- Use GitHub issues for bug reports and feature requests
- Discuss changes in pull requests
- For quick questions, consider using discussions if enabled

### Getting Help
- Check existing documentation and issue tracker
- Look at similar implementations in the codebase
- Ask for clarification in issues if needed
- Be patient and respectful of maintainers' time

### Recognition
- Contributors will be acknowledged in release notes
- Significant contributions may be recognized with maintainership opportunities
- All contributions are valued, regardless of size

## License

By contributing to Phlego, you agree that your contributions will be licensed under the MIT License.

Thank you for contributing to Phlego!