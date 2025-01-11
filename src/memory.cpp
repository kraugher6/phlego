#include "memory.h"
#include "logger.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include <cstring>
#include <regex>
#include <elfio/elfio.hpp>

// Function to convert data based on endianness
uint32_t convert_endianness(uint32_t value, ELFIO::elfio& reader) {
    if (reader.get_encoding() == ELFIO::ELFDATA2LSB) {
        // Little-endian, no conversion needed
        return value;
    } else {
        // Big-endian, convert to little-endian
        return ((value >> 24) & 0x000000FF) |
               ((value >> 8) & 0x0000FF00) |
               ((value << 8) & 0x00FF0000) |
               ((value << 24) & 0xFF000000);
    }
}

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
    ELFIO::elfio reader;

    if (!reader.load(filename)) {
        LOG_ERROR("Error: Failed to open ELF file: " + filename);
        return false;
    }

    // Inizializza il layout della memoria
    for (const auto& segment : reader.segments) {
        if (segment->get_type() == ELFIO::PT_LOAD) {
            uint32_t vaddr = static_cast<uint32_t>(segment->get_virtual_address());
            uint32_t mem_size = static_cast<uint32_t>(segment->get_memory_size());

            if (vaddr < layout.data_start || layout.data_start == 0) {
                layout.text_start = vaddr;
                layout.text_size = mem_size;
            } else if (vaddr >= layout.data_start) {
                layout.data_start = vaddr;
                layout.data_size = mem_size;
            }
        }
    }

    // Ottieni il punto di ingresso
    initial_address = static_cast<uint32_t>(reader.get_entry());

    // Stack pointer: read from ELF if available
    for (const auto& segment : reader.segments) {
        if (segment->get_type() == ELFIO::PT_GNU_STACK) {
            layout.stack_start = static_cast<uint32_t>(segment->get_virtual_address());
            layout.stack_size = static_cast<uint32_t>(segment->get_memory_size());
            break;
        }
    }

    // Default stack pointer if not specified in ELF
    if (layout.stack_start == 0) {
        LOG_INFO("Stack pointer not found in ELF file. Using default stack layout.");
        layout.stack_start = 0x10000;   // Default per ora
        layout.stack_size = 0x1000;      // Default
    }

    // Carica le istruzioni nella memoria
    for (const auto& section : reader.sections) {
        if (section->get_name() == ".text") {
            uint32_t vaddr = static_cast<uint32_t>(section->get_address());
            const char* data_ptr = section->get_data();
            size_t size = section->get_size();

            for (size_t i = 0; i < size; i += 4) {
                uint32_t instruction = *reinterpret_cast<const uint32_t*>(data_ptr + i);
                instruction = convert_endianness(instruction, reader);
                // Store the instruction in memory
                store_word(vaddr + i, instruction);
            }
        }
    }

    LOG_DEBUG("ELF file loaded successfully: " + filename);
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
    LOG_INFO("CPU initialized with program counter set to: 0x" + Memory::to_hex_string(initial_address));
    return initial_address;
}

/**
 * @brief Get the stack pointer address.
 *
 * @return uint32_t The stack pointer address.
 */
uint32_t Memory::get_stack_pointer() const {
    uint32_t sp = layout.stack_start + layout.stack_size;
    LOG_INFO("CPU initialized with stack pointer set to: 0x" + Memory::to_hex_string(sp));
    return sp; // Stack pointer initialized to the top of the stack
    // return 0x10000; // Stack pointer initialized to 0x10000
}

/**
 * @brief Load a byte from memory.
 *
 * @param address Address to load from.
 * @return uint8_t The loaded byte.
 */
uint8_t Memory::load_byte(uint32_t address) const {
    if (address >= data.size()) {
        LOG_ERROR("Memory load address out of range: 0x" + to_hex_string(address));
        throw std::out_of_range("Memory load address out of range");
    }
    return data[address];
}

/**
 * @brief Load a half word from memory.
 *
 * @param address Address to load from.
 * @return uint16_t The loaded half word.
 */
uint16_t Memory::load_half_word(uint32_t address) const {
    if (address + 1 >= data.size()) {
        LOG_ERROR("Memory load address out of range: 0x" + to_hex_string(address));
        throw std::out_of_range("Memory load address out of range");
    }
    return (data[address] << 8) | data[address + 1];
}

/**
 * @brief Load a word from memory.
 *
 * @param address Address to load from.
 * @return uint32_t The loaded word.
 */
uint32_t Memory::load_word(uint32_t address) const {
    if (address + 3 >= data.size()) {
        LOG_ERROR("Memory load address out of range: 0x" + to_hex_string(address));
        throw std::out_of_range("Memory load address out of range");
    }
    return (data[address] << 24) | (data[address + 1] << 16) | (data[address + 2] << 8) | data[address + 3];
}

/**
 * @brief Store a byte in memory.
 *
 * @param address Address to store at.
 * @param value The byte to store.
 */
void Memory::store_byte(uint32_t address, uint8_t value) {
    if (address >= data.size()) {
        LOG_ERROR("Memory store address out of range: 0x" + to_hex_string(address));
        throw std::out_of_range("Memory store address out of range");
    }
    data[address] = value;
}

/**
 * @brief Store a half word in memory.
 *
 * @param address Address to store at.
 * @param value The half word to store.
 */
void Memory::store_half_word(uint32_t address, uint16_t value) {
    if (address + 1 >= data.size()) {
        LOG_ERROR("Memory store address out of range: 0x" + to_hex_string(address));
        throw std::out_of_range("Memory store address out of range");
    }
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
    if (address + 3 >= data.size()) {
        LOG_ERROR("Memory store address out of range: 0x" + to_hex_string(address));
        throw std::out_of_range("Memory store address out of range");
    }
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
        std::cout << "0x" << std::hex << addr << ": 0x" << load_word(addr) << std::endl;
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
    ss << "0x" << std::hex << value;
    return ss.str();
}