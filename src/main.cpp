#include <iostream>
#include "cpu.h"
#include "memory.h"
#include "logger.h"

/**
 * @brief Main function to run the emulator.
 *
 * @param argc Argument count.
 * @param argv Argument vector.
 * @return int Exit status.
 */
int main(int argc, char* argv[]) {
    if (argc != 2) {
        LOG_ERROR("Usage: emulator <log_level> <path_to_elf> <path_to_disassembled>");
        return 1;
    }

    Memory memory(1024 * 1024); // 1 MB of memory
    CPU cpu(memory);

    // if (!memory.load_from_map(argv[1])) {
    //     LOG_ERROR("Failed to load ELF file: " + std::string(argv[2]));
    //     return 1;
    // }

    if (!memory.load_from_elf(argv[1])) {
        LOG_ERROR("Failed to load ELF file: " + std::string(argv[2]));
        return 1;
    }

    // Set the stack pointer after loading the ELF file
    cpu.set_sp(memory.get_stack_pointer());

    // if (!memory.load_from_disassembled(argv[2])) {
    //     LOG_ERROR("Failed to load disassembled file: " + std::string(argv[3]));
    //     return 1;
    // }

    // Set the program counter to the initial address read from the disassembled file
    cpu.set_pc(memory.get_initial_address());

    try {
        cpu.run();
    } catch (const std::exception& e) {
        LOG_ERROR("Error: " + std::string(e.what()));
        return 1;
    }

    return 0;
}

