#include "memory.h"
#include "logger.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include <cstring>
#include <regex>

/**
 * @brief Construct a new Memory object.
 *
 * @param size Size of the memory.
 */
Memory::Memory(size_t size) : data(size), initial_address(0) {
    LOG_DEBUG("Memory initialized with size: " + std::to_string(size) + " bytes.");
}

/**
 * @brief Load memory layout from an ELF file.
 *
 * @param filename Path to the ELF file.
 * @return true if successful, false otherwise.
 */
bool Memory::load_from_elf(const std::string& filename) {
    LOG_DEBUG("Loading ELF file: " + filename);
    // ...existing code...
    return true;
}

/**
 * @brief Load instructions from a disassembled file.
 *
 * @param filename Path to the disassembled file.
 * @return true if successful, false otherwise.
 */
bool Memory::load_from_disassembled(const std::string& filename) {
    std::ifstream file(filename);
    std::string line;

    if (!file.is_open()) {
        LOG_ERROR("Error opening file: " + filename);
        return false;
    }

    LOG_DEBUG("Loading disassembled instructions from file: " + filename);

    // Read the initial address from the file
    while (std::getline(file, line)) {
        if (line.find("<main>:") != std::string::npos) {
            size_t pos = line.find(" ");
            if (pos != std::string::npos) {
                std::string address_hex = line.substr(0, pos);  // Extract the address part
                std::istringstream(address_hex) >> std::hex >> initial_address;
                LOG_DEBUG("Read initial address: 0x" + address_hex);
                break;
            }
        }
    }

    uint32_t address = initial_address;

    // Read the instructions from the file
    while (std::getline(file, line)) {
        // Check if the line contains an instruction (skip header and unnecessary lines)
        if (line.find("<main>") != std::string::npos || line.empty()) {
            continue;
        }

        // Find the part of the line that contains the hexadecimal instruction
        size_t pos = line.find(":");
        if (pos != std::string::npos) {
            // Get the hexadecimal instruction after the colon
            std::string instruction_hex = line.substr(pos + 2, 8);  // Length 8 for the hexadecimal instruction

            // Convert the hexadecimal string to an integer
            uint32_t instruction = 0;
            std::istringstream(instruction_hex) >> std::hex >> instruction;

            // Store the instruction
            store_word(address, instruction);  // Store the instruction in memory

            // Update the address for the next instruction
            address += 4;  // RISC-V has a 4-byte word architecture
        }
    }

    file.close();
    LOG_DEBUG("Successfully loaded disassembled instructions from file: " + filename);
    return true;
}

/**
 * @brief Load memory layout from a map file.
 *
 * @param map_file Path to the map file.
 * @return true if successful, false otherwise.
 */
bool Memory::load_from_map(const std::string& map_file) {
    std::ifstream file(map_file);
    if (!file.is_open()) {
        LOG_ERROR("Error: Cannot open map file: " + map_file);
        return false;
    }

    std::regex section_regex(R"(\.(text|data|bss|stack)\s+0x([0-9a-fA-F]+)\s+0x([0-9a-fA-F]+))");
    std::string line;
    while (std::getline(file, line)) {
        std::smatch match;
        if (std::regex_search(line, match, section_regex)) {
            const std::string& section_name = match[1];
            uint32_t start = std::stoul(match[2], nullptr, 16);
            uint32_t size = std::stoul(match[3], nullptr, 16);

            if (section_name == "text") {
                layout.text_start = start;
                layout.text_size = size;
            } else if (section_name == "data") {
                layout.data_start = start;
                layout.data_size = size;
            } else if (section_name == "bss") {
                layout.bss_start = start;
                layout.bss_size = size;
            } else if (section_name == "stack") {
                layout.stack_start = start;
                layout.stack_size = size;
            }
        }
    }

    file.close();
    LOG_DEBUG("Memory layout loaded from map file:");
    layout.print();
    return true;
}

/**
 * @brief Get the initial address read from the disassembled file.
 *
 * @return uint32_t The initial address.
 */
uint32_t Memory::get_initial_address() const {
    return initial_address;
}

/**
 * @brief Get the stack pointer address.
 *
 * @return uint32_t The stack pointer address.
 */
uint32_t Memory::get_stack_pointer() const {
    // return layout.stack_start + layout.stack_size; // Stack pointer initialized to the top of the stack
    return 0x10000; // Stack pointer initialized to 0x10000
}

/**
 * @brief Load a byte from memory.
 *
 * @param address Address to load from.
 * @return uint8_t The loaded byte.
 */
uint8_t Memory::load_byte(uint32_t address) const {
    return data[address];
}

/**
 * @brief Load a half word from memory.
 *
 * @param address Address to load from.
 * @return uint16_t The loaded half word.
 */
uint16_t Memory::load_half_word(uint32_t address) const {
    return (data[address] << 8) | data[address + 1];
}

/**
 * @brief Load a word from memory.
 *
 * @param address Address to load from.
 * @return uint32_t The loaded word.
 */
uint32_t Memory::load_word(uint32_t address) const {
    return (data[address] << 24) | (data[address + 1] << 16) | (data[address + 2] << 8) | data[address + 3];
}

/**
 * @brief Store a byte in memory.
 *
 * @param address Address to store at.
 * @param value The byte to store.
 */
void Memory::store_byte(uint32_t address, uint8_t value) {
    data[address] = value;
}

/**
 * @brief Store a half word in memory.
 *
 * @param address Address to store at.
 * @param value The half word to store.
 */
void Memory::store_half_word(uint32_t address, uint16_t value) {
    data[address] = value >> 8;
    data[address + 1] = value & 0xFF;
}

/**
 * @brief Store a word in memory.
 *
 * @param address Address to store at.
 * @param value The word to store.
 */
void Memory::store_word(uint32_t address, uint32_t value) {
    data[address] = value >> 24;
    data[address + 1] = (value >> 16) & 0xFF;
    data[address + 2] = (value >> 8) & 0xFF;
    data[address + 3] = value & 0xFF;
}

/**
 * @brief Print the memory contents.
 *
 * @param start_address Start address of the memory region to print.
 * @param end_address End address of the memory region to print.
 */
void Memory::print_memory(uint32_t start_address, uint32_t end_address) const {
    std::cout << "Memory state (0x" << std::hex << start_address << " - 0x" << end_address << "):" << std::endl;
    for (uint32_t addr = start_address; addr < end_address; addr += 4) {
        uint32_t value = load_word(addr);
        std::cout << "0x" << std::hex << addr << ": 0x" << value << std::endl;
    }
}

/**
 * @brief Convert a value to a hexadecimal string.
 *
 * @param value The value to convert.
 * @return std::string The hexadecimal string.
 */
std::string Memory::to_hex_string(uint32_t value) {
    std::stringstream ss;
    ss << std::hex << value;
    return ss.str();
}