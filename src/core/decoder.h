#ifndef DECODER_H
#define DECODER_H

#include <cstdint>

namespace phlego {

enum class InstructionType {
    R_TYPE, I_TYPE, S_TYPE, B_TYPE, U_TYPE, J_TYPE, UNKNOWN
};

struct DecodedInstruction {
    InstructionType type = InstructionType::UNKNOWN;
    uint8_t opcode = 0;
    uint8_t rd = 0;
    uint8_t rs1 = 0;
    uint8_t rs2 = 0;
    uint8_t funct3 = 0;
    uint8_t funct7 = 0;
    int32_t imm = 0;
    uint32_t raw = 0;
};

class Decoder {
public:
    static DecodedInstruction decode(uint32_t instruction);
};

} // namespace phlego

#endif
