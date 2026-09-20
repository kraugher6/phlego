#include "core/register_file.h"
#include "core/memory.h"
#include <iostream>
#include <iomanip>
#include <sstream>
#include "logger.h"

namespace phlego {

const std::array<std::string, 32> RegisterFile::reg_names = {
    "zero", "ra", "sp", "gp", "tp", "t0", "t1", "t2",
    "s0/fp", "s1", "a0", "a1", "a2", "a3", "a4", "a5",
    "a6", "a7", "s2", "s3", "s4", "s5", "s6", "s7",
    "s8", "s9", "s10", "s11", "t3", "t4", "t5", "t6"
};

RegisterFile::RegisterFile() {
    registers.fill(0);
}

uint32_t RegisterFile::read(uint8_t reg) const {
    if (reg == 0) return 0;
    if (reg >= 32) return 0;
    return registers[reg];
}

void RegisterFile::write(uint8_t reg, uint32_t val) {
    if (reg == 0) return; // x0 is always 0
    if (reg >= 32) return;
    LOG_DEBUG("Write x" + std::to_string(reg) + " = 0x" + Memory::to_hex_string(val));
    registers[reg] = val;
}

void RegisterFile::dump() const {
    LOG_INFO("--- Register Dump ---");
    for (int i = 0; i < 32; ++i) {
        std::stringstream ss;
        ss << std::left << std::setw(6) << reg_names[i] 
           << " (x" << std::dec << i << "): 0x" 
           << Memory::to_hex_string(registers[i]);
        LOG_INFO(ss.str());
    }
}

} // namespace phlego
