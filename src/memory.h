#ifndef MEMORY_H
#define MEMORY_H

#include <string>
#include <vector>
#include <cstddef> // Necessary for size_t type
#include <iostream> // Necessary for std::cout and std::hex

struct MemoryLayout {
    uint32_t text_start;
    uint32_t text_size;
    uint32_t data_start;
    uint32_t data_size;
    uint32_t bss_start;
    uint32_t bss_size;
    uint32_t heap_start;
    uint32_t stack_start;
    uint32_t stack_size;

    void print() const {
        std::cout << "Text: Start=0x" << std::hex << text_start << " Size=0x" << text_size << '\n';
        std::cout << "Data: Start=0x" << std::hex << data_start << " Size=0x" << data_size << '\n';
        std::cout << "BSS: Start=0x" << std::hex << bss_start << " Size=0x" << bss_size << '\n';
        std::cout << "Heap: Start=0x" << std::hex << heap_start << '\n';
        std::cout << "Stack: Start=0x" << std::hex << stack_start << " Size=0x" << stack_size << '\n';
    }
};

class Memory {
public:
    Memory(size_t size);
    bool load_from_elf(const std::string& filename); // New function to load binary files
    bool load_from_disassembled(const std::string& filename); // Updated function to return success status
    bool load_from_map(const std::string& map_file); // New function to load memory layout from map file
    uint8_t load_byte(uint32_t address) const;
    uint16_t load_half_word(uint32_t address) const;
    uint32_t load_word(uint32_t address) const;
    void store_byte(uint32_t address, uint8_t value);
    void store_half_word(uint32_t address, uint16_t value);
    void store_word(uint32_t address, uint32_t value);
    void print_memory(uint32_t start_address, uint32_t end_address) const;
    static std::string to_hex_string(uint32_t value); // Add this line

    /**
     * @brief Get the initial address read from the disassembled file.
     *
     * @return uint32_t The initial address.
     */
    uint32_t get_initial_address() const;

    /**
     * @brief Get the memory layout.
     *
     * @return MemoryLayout The memory layout.
     */
    MemoryLayout get_memory_layout() const;

    /**
     * @brief Read the stack pointer from the memory layout.
     *
     * @return uint32_t The stack pointer address.
     */
    uint32_t get_stack_pointer() const;

private:
    std::vector<uint8_t> data;
    uint32_t initial_address; ///< Initial address read from the disassembled file.
    MemoryLayout layout; ///< Memory layout.
};

#endif
