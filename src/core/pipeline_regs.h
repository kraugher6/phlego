#ifndef PIPELINE_REGS_H
#define PIPELINE_REGS_H

#include <cstdint>
#include <array>
#include "core/decoder.h"

namespace phlego {

const int MAX_WIDTH = 2;

struct IF_ID_Entry {
    bool valid = false;
    uint32_t pc = 0;
    uint32_t instr = 0;
    bool pred_taken = false;
    uint32_t pred_target = 0;
};

struct IF_ID {
    std::array<IF_ID_Entry, MAX_WIDTH> slots;
    uint32_t count = 0;
    void clear() { slots.fill({}); count = 0; }
};

struct ID_EX_Entry {
    bool valid = false;
    uint32_t pc = 0;
    DecodedInstruction dec;
    uint32_t rs1_val = 0;
    uint32_t rs2_val = 0;
    bool pred_taken = false;
    uint32_t pred_target = 0;
};

struct ID_EX {
    std::array<ID_EX_Entry, MAX_WIDTH> slots;
    uint32_t count = 0;
    void clear() { slots.fill({}); count = 0; }
};

struct EX_MEM_Entry {
    bool valid = false;
    uint32_t pc = 0;
    uint8_t rd = 0;
    uint32_t alu_result = 0;
    uint32_t rs2_val = 0; 
    DecodedInstruction dec;
};

struct EX_MEM {
    std::array<EX_MEM_Entry, MAX_WIDTH> slots;
    uint32_t count = 0;
    void clear() { slots.fill({}); count = 0; }
};

struct MEM_WB_Entry {
    bool valid = false;
    uint32_t pc = 0;
    uint8_t rd = 0;
    uint32_t result = 0;
    DecodedInstruction dec;
};

struct MEM_WB {
    std::array<MEM_WB_Entry, MAX_WIDTH> slots;
    uint32_t count = 0;
    void clear() { slots.fill({}); count = 0; }
};

} // namespace phlego

#endif
