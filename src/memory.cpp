#include "memory.h"

#include <cstring>
#include <elfio/elfio.hpp>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <regex>
#include <sstream>

#include "logger.h"

/**
 * @brief Convert a 32-bit value based on endianness.
 *
 * Converts a 32-bit value to little-endian format if the ELF file is in
 * big-endian format.
 *
 * @param value The 32-bit value to convert.
 * @param reader Reference to the ELF reader object.
 * @return uint32_t The converted value.
 */
uint32_t convert_endianness(uint32_t value, ELFIO::elfio &reader) {
    if (reader.get_encoding() == ELFIO::ELFDATA2LSB) {
        // Little-endian, no conversion needed
        return value;
    } else {
        // Big-endian, convert to little-endian
        return ((value >> 24) & 0x000000FF) | ((value >> 8) & 0x0000FF00) |
               ((value << 8) & 0x00FF0000) | ((value << 24) & 0xFF000000);
    }
}

/**
 * @brief Construct a new Memory object.
 *
 * Initializes the memory with the specified size and sets the initial address
 * to zero.
 *
 * @param size Size of the memory in bytes.
 */
Memory::Memory(size_t size) : data(size), initial_address(0) {
    LOG_DEBUG("Memory initialized with size: " + std::to_string(size) +
              " bytes.");
}

/**
 * @brief Load memory layout and instructions from an ELF file.
 *
 * Parses the ELF file to extract memory layout, entry point, and instructions.
 *
 * @param filename Path to the ELF file.
 * @return true if the ELF file is successfully loaded, false otherwise.
 */
bool Memory::load_from_elf(const std::string &filename) {
    LOG_DEBUG("Loading ELF file: " + filename);
    ELFIO::elfio reader;

    if (!reader.load(filename)) {
        LOG_ERROR("Error: Failed to open ELF file: " + filename);
        return false;
    }

    // Initialize memory layout
    for (const auto &segment : reader.segments) {
        if (segment->get_type() == ELFIO::PT_LOAD) {
            uint32_t vaddr =
                static_cast<uint32_t>(segment->get_virtual_address());
            uint32_t mem_size =
                static_cast<uint32_t>(segment->get_memory_size());

            if (vaddr < layout.data_start || layout.data_start == 0) {
                layout.text_start = vaddr;
                layout.text_size = mem_size;
            } else if (vaddr >= layout.data_start) {
                layout.data_start = vaddr;
                layout.data_size = mem_size;
            }
        }
    }

    // Get the entry point
    initial_address = static_cast<uint32_t>(reader.get_entry());

    // Parse stack pointer from ELF if available
    for (const auto &segment : reader.segments) {
        if (segment->get_type() == ELFIO::PT_GNU_STACK) {
            layout.stack_start =
                static_cast<uint32_t>(segment->get_virtual_address());
            layout.stack_size =
                static_cast<uint32_t>(segment->get_memory_size());
            break;
        }
    }

    // Default stack pointer if not specified in ELF
    if (layout.stack_start == 0) {
        LOG_INFO(
            "Stack pointer not found in ELF file. Using default stack layout.");
        layout.stack_start = 0x10000;  // Default stack start
        layout.stack_size = 0x1000;    // Default stack size
    }

    // Load instructions into memory
    for (const auto &section : reader.sections) {
        if (section->get_name() == ".text") {
            uint32_t vaddr = static_cast<uint32_t>(section->get_address());
            const char *data_ptr = section->get_data();
            size_t size = section->get_size();

            for (size_t i = 0; i < size; i += 4) {
                uint32_t instruction =
                    *reinterpret_cast<const uint32_t *>(data_ptr + i);
                instruction = convert_endianness(instruction, reader);
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
 * Parses a disassembled file to extract instructions and their addresses.
 *
 * @param filename Path to the disassembled file.
 * @return true if the file is successfully loaded, false otherwise.
 */
bool Memory::load_from_disassembled(const std::string &filename) {
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
                std::string address_hex = line.substr(0, pos);
                std::istringstream(address_hex) >> std::hex >> initial_address;
                LOG_DEBUG("Read initial address: 0x" + address_hex);
                break;
            }
        }
    }

    uint32_t address = initial_address;

    // Read the instructions from the file
    while (std::getline(file, line)) {
        if (line.find("<main>") != std::string::npos || line.empty()) {
            continue;
        }

        size_t pos = line.find(":");
        if (pos != std::string::npos) {
            std::string instruction_hex = line.substr(pos + 2, 8);
            uint32_t instruction = 0;
            std::istringstream(instruction_hex) >> std::hex >> instruction;
            store_word(address, instruction);
            address += 4;
        }
    }

    file.close();
    LOG_DEBUG("Successfully loaded disassembled instructions from file: " +
              filename);
    return true;
}

/**
 * @brief Load memory layout from a map file.
 *
 * Parses a map file to extract memory layout information for sections like
 * text, data, bss, and stack.
 *
 * @param map_file Path to the map file.
 * @return true if the map file is successfully loaded, false otherwise.
 */
bool Memory::load_from_map(const std::string &map_file) {
    std::ifstream file(map_file);
    if (!file.is_open()) {
        LOG_ERROR("Error: Cannot open map file: " + map_file);
        return false;
    }

    std::regex section_regex(
        R"(\.(text|data|bss|stack)\s+0x([0-9a-fA-F]+)\s+0x([0-9a-fA-F]+))");
    std::string line;
    while (std::getline(file, line)) {
        std::smatch match;
        if (std::regex_search(line, match, section_regex)) {
            const std::string &section_name = match[1];
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
 * @brief Get the initial address read from the ELF or disassembled file.
 *
 * @return uint32_t The initial address.
 */
uint32_t Memory::get_initial_address() const {
    LOG_INFO("CPU initialized with program counter set to: 0x" +
             Memory::to_hex_string(initial_address));
    return initial_address;
}

/**
 * @brief Get the stack pointer address.
 *
 * Calculates the stack pointer address based on the stack layout.
 *
 * @return uint32_t The stack pointer address.
 */
uint32_t Memory::get_stack_pointer() const {
    uint32_t sp = layout.stack_start + layout.stack_size;
    LOG_INFO("CPU initialized with stack pointer set to: 0x" +
             Memory::to_hex_string(sp));
    return sp;
}

/**
 * @brief Load a byte from memory.
 *
 * Reads a single byte from the specified memory address.
 *
 * @param address Address to load from.
 * @return uint8_t The loaded byte.
 * @throws std::out_of_range if the address is out of bounds.
 */
uint8_t Memory::load_byte(uint32_t address) const {
    if (address >= data.size()) {
        LOG_ERROR("Memory load address out of range: 0x" +
                  to_hex_string(address));
        throw std::out_of_range("Memory load address out of range");
    }
    return data[address];
}

/**
 * @brief Load a half word from memory.
 *
 * Reads two bytes from the specified memory address and combines them into a
 * 16-bit value.
 *
 * @param address Address to load from.
 * @return uint16_t The loaded half word.
 * @throws std::out_of_range if the address is out of bounds.
 */
uint16_t Memory::load_half_word(uint32_t address) const {
    if (address + 1 >= data.size()) {
        LOG_ERROR("Memory load address out of range: 0x" +
                  to_hex_string(address));
        throw std::out_of_range("Memory load address out of range");
    }
    return (data[address] << 8) | data[address + 1];
}

/**
 * @brief Load a word from memory.
 *
 * Reads four bytes from the specified memory address and combines them into a
 * 32-bit value.
 *
 * @param address Address to load from.
 * @return uint32_t The loaded word.
 * @throws std::out_of_range if the address is out of bounds.
 */
uint32_t Memory::load_word(uint32_t address) const {
    if (address + 3 >= data.size()) {
        LOG_ERROR("Memory load address out of range: 0x" +
                  to_hex_string(address));
        throw std::out_of_range("Memory load address out of range");
    }
    return (data[address] << 24) | (data[address + 1] << 16) |
           (data[address + 2] << 8) | data[address + 3];
}

/**
 * @brief Store a byte in memory.
 *
 * Writes a single byte to the specified memory address.
 *
 * @param address Address to store at.
 * @param value The byte to store.
 * @throws std::out_of_range if the address is out of bounds.
 */
void Memory::store_byte(uint32_t address, uint8_t value) {
    if (address >= data.size()) {
        LOG_ERROR("Memory store address out of range: 0x" +
                  to_hex_string(address));
        throw std::out_of_range("Memory store address out of range");
    }
    data[address] = value;
}

/**
 * @brief Store a half word in memory.
 *
 * Writes two bytes to the specified memory address.
 *
 * @param address Address to store at.
 * @param value The half word to store.
 * @throws std::out_of_range if the address is out of bounds.
 */
void Memory::store_half_word(uint32_t address, uint16_t value) {
    if (address + 1 >= data.size()) {
        LOG_ERROR("Memory store address out of range: 0x" +
                  to_hex_string(address));
        throw std::out_of_range("Memory store address out of range");
    }
    data[address] = value >> 8;
    data[address + 1] = value & 0xFF;
}

/**
 * @brief Store a word in memory.
 *
 * Writes four bytes to the specified memory address.
 *
 * @param address Address to store at.
 * @param value The word to store.
 * @throws std::out_of_range if the address is out of bounds.
 */
void Memory::store_word(uint32_t address, uint32_t value) {
    if (address + 3 >= data.size()) {
        LOG_ERROR("Memory store address out of range: 0x" +
                  to_hex_string(address));
        throw std::out_of_range("Memory store address out of range");
    }
    data[address] = value >> 24;
    data[address + 1] = (value >> 16) & 0xFF;
    data[address + 2] = (value >> 8) & 0xFF;
    data[address + 3] = value & 0xFF;
}

/**
 * @brief Get the memory layout.
 *
 * Retrieves the memory layout, including text, data, bss, and stack sections.
 *
 * @return MemoryLayout The memory layout.
 */
MemoryLayout Memory::get_memory_layout() const { return layout; }

/**
 * @brief Print the memory contents.
 *
 * Outputs the contents of the specified memory range to the console.
 *
 * @param start_address Start address of the memory region to print.
 * @param end_address End address of the memory region to print.
 */
void Memory::print_memory(uint32_t start_address, uint32_t end_address) const {
    std::cout << "Memory state (0x" << std::hex << start_address << " - 0x"
              << end_address << "):" << std::endl;
    for (uint32_t addr = start_address; addr < end_address; addr += 4) {
        std::cout << "0x" << std::hex << addr << ": 0x" << load_word(addr)
                  << std::endl;
    }
}

/**
 * @brief Convert a value to a hexadecimal string.
 *
 * Converts a 32-bit value to a string in hexadecimal format.
 *
 * @param value The value to convert.
 * @return std::string The hexadecimal string representation of the value.
 */
std::string Memory::to_hex_string(uint32_t value) {
    std::stringstream ss;
    ss << "0x" << std::hex << std::setw(8) << std::setfill('0') << value;
    return ss.str();
}