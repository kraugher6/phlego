#ifndef MEMORY_H
#define MEMORY_H

#include <vector>
#include <cstdint>
#include <string>
#include <sstream>
#include <iomanip>
#include "utils/elf_loader.h"
#include "logger.h"

namespace phlego {

struct MemoryLayout {
    uint32_t text_start;
    uint32_t text_size;
    uint32_t data_start;
    uint32_t data_size;
    uint32_t bss_start;
    uint32_t bss_size;
    uint32_t stack_start;
    uint32_t stack_size;

    void print() const {
        auto to_hex = [](uint32_t v) {
            std::ostringstream ss;
            ss << "0x" << std::hex << std::setw(8) << std::setfill('0') << v;
            return ss.str();
        };
        LOG_INFO("Text: Start=" + to_hex(text_start) + " Size=" + to_hex(text_size));
        LOG_INFO("Data: Start=" + to_hex(data_start) + " Size=" + to_hex(data_size));
        LOG_INFO("Stack: Start=" + to_hex(stack_start) + " Size=" + to_hex(stack_size));
    }
};

class Memory {
public:
    Memory(uint32_t size = 128 * 1024 * 1024);

    uint8_t  read_byte(uint32_t addr) const;
    uint16_t read_half(uint32_t addr) const;
    uint32_t read_word(uint32_t addr) const;

    void write_byte(uint32_t addr, uint8_t  val);
    void write_half(uint32_t addr, uint16_t val);
    void write_word(uint32_t addr, uint32_t val);

    bool load_elf(const std::string& filename, uint32_t& entry_point);

    void dump(uint32_t start, uint32_t size) const;

    MemoryLayout get_memory_layout() const { return layout; }
    static std::string to_hex_string(uint32_t value);

private:
    std::vector<uint8_t> data;
    uint32_t size;
    MemoryLayout layout;

    bool check_bounds(uint32_t addr, uint32_t access_size) const;
};

} // namespace phlego

#endif
