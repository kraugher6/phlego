#ifndef MEMORY_H
#define MEMORY_H

#include <string>
#include <vector>
#include <cstddef> // Necessary for size_t type
#include <iostream> // Necessary for std::cout and std::hex

/**
 * @brief Struct representing the memory layout.
 */
struct MemoryLayout {
    uint32_t text_start; ///< Start address of the text section.
    uint32_t text_size; ///< Size of the text section.
    uint32_t data_start; ///< Start address of the data section.
    uint32_t data_size; ///< Size of the data section.
    uint32_t bss_start; ///< Start address of the BSS section.
    uint32_t bss_size; ///< Size of the BSS section.
    uint32_t heap_start; ///< Start address of the heap.
    uint32_t stack_start; ///< Start address of the stack.
    uint32_t stack_size; ///< Size of the stack.

    /**
     * @brief Print the memory layout.
     */
    void print() const {
        std::cout << "Text: Start=0x" << std::hex << text_start << " Size=0x" << text_size << '\n';
        std::cout << "Data: Start=0x" << std::hex << data_start << " Size=0x" << data_size << '\n';
        std::cout << "BSS: Start=0x" << std::hex << bss_start << " Size=0x" << bss_size << '\n';
        std::cout << "Heap: Start=0x" << std::hex << heap_start << '\n';
        std::cout << "Stack: Start=0x" << std::hex << stack_start << " Size=0x" << stack_size << '\n';
    }
};

/**
 * @brief Class representing the memory.
 */
class Memory {
public:
    /**
     * @brief Construct a new Memory object.
     *
     * @param size Size of the memory.
     */
    Memory(size_t size);

    /**
     * @brief Load memory layout from an ELF file.
     *
     * @param filename Path to the ELF file.
     * @return true if successful, false otherwise.
     */
    bool load_from_elf(const std::string& filename);

    /**
     * @brief Load instructions from a disassembled file.
     *
     * @param filename Path to the disassembled file.
     * @return true if successful, false otherwise.
     */
    bool load_from_disassembled(const std::string& filename);

    /**
     * @brief Load memory layout from a map file.
     *
     * @param map_file Path to the map file.
     * @return true if successful, false otherwise.
     */
    bool load_from_map(const std::string& map_file);

    /**
     * @brief Load a byte from memory.
     *
     * @param address Address to load from.
     * @return uint8_t The loaded byte.
     */
    uint8_t load_byte(uint32_t address) const;

    /**
     * @brief Load a half word from memory.
     *
     * @param address Address to load from.
     * @return uint16_t The loaded half word.
     */
    uint16_t load_half_word(uint32_t address) const;

    /**
     * @brief Load a word from memory.
     *
     * @param address Address to load from.
     * @return uint32_t The loaded word.
     */
    uint32_t load_word(uint32_t address) const;

    /**
     * @brief Store a byte in memory.
     *
     * @param address Address to store at.
     * @param value The byte to store.
     */
    void store_byte(uint32_t address, uint8_t value);

    /**
     * @brief Store a half word in memory.
     *
     * @param address Address to store at.
     * @param value The half word to store.
     */
    void store_half_word(uint32_t address, uint16_t value);

    /**
     * @brief Store a word in memory.
     *
     * @param address Address to store at.
     * @param value The word to store.
     */
    void store_word(uint32_t address, uint32_t value);

    /**
     * @brief Print the memory contents.
     *
     * @param start_address Start address of the memory region to print.
     * @param end_address End address of the memory region to print.
     */
    void print_memory(uint32_t start_address, uint32_t end_address) const;

    /**
     * @brief Convert a value to a hexadecimal string.
     *
     * @param value The value to convert.
     * @return std::string The hexadecimal string.
     */
    static std::string to_hex_string(uint32_t value);

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
    std::vector<uint8_t> data; ///< Memory data.
    uint32_t initial_address; ///< Initial address read from the disassembled file.
    MemoryLayout layout; ///< Memory layout.
};

#endif
