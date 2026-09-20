#include "core/decoder.h"

namespace phlego {

DecodedInstruction Decoder::decode(uint32_t instr) {
    DecodedInstruction dec;
    dec.raw = instr;
    dec.opcode = instr & 0x7F;

    switch (dec.opcode) {
        case 0x33: // R-type
            dec.type = InstructionType::R_TYPE;
            dec.rd = (instr >> 7) & 0x1F;
            dec.funct3 = (instr >> 12) & 0x07;
            dec.rs1 = (instr >> 15) & 0x1F;
            dec.rs2 = (instr >> 20) & 0x1F;
            dec.funct7 = (instr >> 25) & 0x7F;
            break;

        case 0x13: // I-type ALU
        case 0x03: // I-type Load
        case 0x67: // JALR
            dec.type = InstructionType::I_TYPE;
            dec.rd = (instr >> 7) & 0x1F;
            dec.funct3 = (instr >> 12) & 0x07;
            dec.rs1 = (instr >> 15) & 0x1F;
            dec.imm = static_cast<int32_t>(instr) >> 20; // Sign extend
            break;

        case 0x23: // S-type Store
            dec.type = InstructionType::S_TYPE;
            dec.funct3 = (instr >> 12) & 0x07;
            dec.rs1 = (instr >> 15) & 0x1F;
            dec.rs2 = (instr >> 20) & 0x1F;
            dec.imm = static_cast<int32_t>((instr & 0xFE000000) >> 20) | ((instr & 0x00000F80) >> 7);
            // Re-sign extend 12-bit immediate
            if (dec.imm & 0x800) dec.imm |= 0xFFFFF000;
            break;

        case 0x63: // B-type Branch
            dec.type = InstructionType::B_TYPE;
            dec.funct3 = (instr >> 12) & 0x07;
            dec.rs1 = (instr >> 15) & 0x1F;
            dec.rs2 = (instr >> 20) & 0x1F;
            dec.imm = ((instr & 0x80000000) >> 19) | // bit 12
                      ((instr & 0x7E000000) >> 20) | // bits 10:5
                      ((instr & 0x00000F00) >> 7)  | // bits 4:1
                      ((instr & 0x00000080) << 4);   // bit 11
            // Re-sign extend 13-bit immediate (bit 0 is always 0)
            if (dec.imm & 0x1000) dec.imm |= 0xFFFFE000;
            break;

        case 0x37: // LUI
        case 0x17: // AUIPC
            dec.type = InstructionType::U_TYPE;
            dec.rd = (instr >> 7) & 0x1F;
            dec.imm = instr & 0xFFFFF000;
            break;

        case 0x6F: // JAL
            dec.type = InstructionType::J_TYPE;
            dec.rd = (instr >> 7) & 0x1F;
            dec.imm = ((instr & 0x80000000) >> 11) | // bit 20
                      ((instr & 0x7FE00000) >> 20) | // bits 10:1
                      ((instr & 0x00100000) >> 9)  | // bit 11
                      (instr & 0x000FF000);          // bits 19:12
            // Re-sign extend 21-bit immediate (bit 0 is always 0)
            if (dec.imm & 0x100000) dec.imm |= 0xFFE00000;
            break;

        default:
            dec.type = InstructionType::UNKNOWN;
            break;
    }

    return dec;
}

} // namespace phlego
