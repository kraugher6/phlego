#include "core/memory.h"
#include <iostream>
#include <iomanip>
#include <cstring>
#include <sstream>
#include "logger.h"

namespace phlego {

Memory::Memory(uint32_t mem_size) : size(mem_size) {
    data.resize(size, 0);
    layout = {};
}

bool Memory::check_bounds(uint32_t addr, uint32_t access_size) const {
    if (addr + access_size > size) {
        LOG_ERROR("Memory out of bounds access at 0x" + to_hex_string(addr));
        return false;
    }
    return true;
}

uint8_t Memory::read_byte(uint32_t addr) const {
    if (!check_bounds(addr, 1)) return 0;
    return data[addr];
}

uint16_t Memory::read_half(uint32_t addr) const {
    if (!check_bounds(addr, 2)) return 0;
    // RISC-V is little-endian: low byte at low address
    return (uint16_t)data[addr] | ((uint16_t)data[addr + 1] << 8);
}

uint32_t Memory::read_word(uint32_t addr) const {
    if (!check_bounds(addr, 4)) return 0;
    // RISC-V is little-endian
    return (uint32_t)data[addr] | 
           ((uint32_t)data[addr + 1] << 8) | 
           ((uint32_t)data[addr + 2] << 16) | 
           ((uint32_t)data[addr + 3] << 24);
}

void Memory::write_byte(uint32_t addr, uint8_t val) {
    if (!check_bounds(addr, 1)) return;
    data[addr] = val;
}

void Memory::write_half(uint32_t addr, uint16_t val) {
    if (!check_bounds(addr, 2)) return;
    data[addr] = val & 0xFF;
    data[addr + 1] = (val >> 8) & 0xFF;
}

void Memory::write_word(uint32_t addr, uint32_t val) {
    if (!check_bounds(addr, 4)) return;
    data[addr] = val & 0xFF;
    data[addr + 1] = (val >> 8) & 0xFF;
    data[addr + 2] = (val >> 16) & 0xFF;
    data[addr + 3] = (val >> 24) & 0xFF;
}

bool Memory::load_elf(const std::string& filename, uint32_t& entry_point) {
    ElfLoader loader(filename);
    if (!loader.load()) return false;

    entry_point = loader.get_entry_point();

    for (const auto& segment : loader.get_segments()) {
        if (segment.vaddr + segment.memsz > size) {
            LOG_ERROR("ELF segment out of memory bounds: vaddr=0x" + to_hex_string(segment.vaddr) + " memsz=0x" + to_hex_string(segment.memsz));
            return false;
        }

        // Copy data from file
        std::memcpy(&data[segment.vaddr], segment.data.data(), segment.data.size());

        // Zero out the rest (BSS)
        if (segment.memsz > segment.data.size()) {
            std::memset(&data[segment.vaddr + segment.data.size()], 0, segment.memsz - segment.data.size());
        }

        // Update layout
        if (segment.flags & 1) { // PF_X (Executable)
            layout.text_start = segment.vaddr;
            layout.text_size = segment.memsz;
        } else {
            if (layout.data_start == 0) {
                layout.data_start = segment.vaddr;
                layout.data_size = segment.memsz;
            }
        }
    }

    LOG_DEBUG("Memory layout updated after ELF load:");
    layout.print();
    return true;
}

void Memory::dump(uint32_t start, uint32_t dump_size) const {
    LOG_INFO("--- Memory Dump [0x" + to_hex_string(start) + " - 0x" + to_hex_string(start + dump_size) + "] ---");
    for (uint32_t addr = start; addr < start + dump_size; addr += 16) {
        std::stringstream ss;
        ss << std::hex << std::setw(8) << std::setfill('0') << addr << ": ";
        for (int i = 0; i < 16 && (addr + i < start + dump_size); ++i) {
            ss << std::hex << std::setw(2) << std::setfill('0') << (int)data[addr + i] << " ";
        }
        LOG_INFO(ss.str());
    }
}

std::string Memory::to_hex_string(uint32_t value) {
    std::stringstream ss;
    ss << std::hex << std::setw(8) << std::setfill('0') << value;
    return ss.str();
}

} // namespace phlego
