#include "core/interpreter.h"
#include <iostream>
#include <iomanip>
#include <sstream>
#include "logger.h"

namespace phlego {

Interpreter::Interpreter(Memory& mem) 
    : memory(mem), pc(0), inst_count(0), halted(false) {}

bool Interpreter::step() {
    if (halted) return false;

    // Fetch
    uint32_t raw_instr = memory.read_word(pc);
    LOG_DEBUG("Fetch PC: 0x" + Memory::to_hex_string(pc) + " Raw: 0x" + Memory::to_hex_string(raw_instr));
    
    // Quick check for ECALL/EBREAK
    if (raw_instr == 0x00000073) {
        halted = true;
        LOG_INFO("Execution halted by ECALL/EBREAK at PC: 0x" + std::to_string(pc));
        return false;
    }

    // Decode
    DecodedInstruction instr = Decoder::decode(raw_instr);
    if (instr.type == InstructionType::UNKNOWN) {
        LOG_ERROR("Unknown instruction 0x" + std::to_string(raw_instr) + " at PC 0x" + std::to_string(pc));
        halted = true;
        return false;
    }

    // Execute
    execute(instr);

    inst_count++;
    return !halted;
}

void Interpreter::run() {
    LOG_INFO("Starting execution loop...");
    while (step());
    LOG_INFO("Simulation finished.");
    LOG_INFO("Total instructions: " + std::to_string(inst_count));
}

void Interpreter::execute(const DecodedInstruction& instr) {
    uint32_t next_pc = pc + 4;
    
    // RV32I Implementation
    switch (instr.opcode) {
        case 0x37: // LUI
            rf.write(instr.rd, instr.imm);
            break;
        case 0x17: // AUIPC
            rf.write(instr.rd, pc + instr.imm);
            break;
        case 0x6F: // JAL
            rf.write(instr.rd, pc + 4);
            next_pc = pc + instr.imm;
            break;
        case 0x67: // JALR
            rf.write(instr.rd, pc + 4);
            next_pc = (rf.read(instr.rs1) + instr.imm) & ~1;
            break;
        case 0x63: // B-type
        {
            uint32_t val1 = rf.read(instr.rs1);
            uint32_t val2 = rf.read(instr.rs2);
            bool take = false;
            switch (instr.funct3) {
                case 0: take = (val1 == val2); break; // BEQ
                case 1: take = (val1 != val2); break; // BNE
                case 4: take = ((int32_t)val1 < (int32_t)val2); break; // BLT
                case 5: take = ((int32_t)val1 >= (int32_t)val2); break; // BGE
                case 6: take = (val1 < val2); break; // BLTU
                case 7: take = (val1 >= val2); break; // BGEU
            }
            if (take) next_pc = pc + instr.imm;
            break;
        }
        case 0x03: // Load
        {
            uint32_t addr = rf.read(instr.rs1) + instr.imm;
            switch (instr.funct3) {
                case 0: rf.write(instr.rd, (int32_t)(int8_t)memory.read_byte(addr)); break; // LB
                case 1: rf.write(instr.rd, (int32_t)(int16_t)memory.read_half(addr)); break; // LH
                case 2: rf.write(instr.rd, memory.read_word(addr)); break; // LW
                case 4: rf.write(instr.rd, memory.read_byte(addr)); break; // LBU
                case 5: rf.write(instr.rd, memory.read_half(addr)); break; // LHU
            }
            break;
        }
        case 0x23: // Store
        {
            uint32_t addr = rf.read(instr.rs1) + instr.imm;
            uint32_t val = rf.read(instr.rs2);
            switch (instr.funct3) {
                case 0: memory.write_byte(addr, val & 0xFF); break; // SB
                case 1: memory.write_half(addr, val & 0xFFFF); break; // SH
                case 2: memory.write_word(addr, val); break; // SW
            }
            break;
        }
        case 0x13: // I-type ALU
        {
            uint32_t val1 = rf.read(instr.rs1);
            uint32_t imm = instr.imm;
            uint32_t res = 0;
            switch (instr.funct3) {
                case 0: res = val1 + imm; break; // ADDI
                case 2: res = ((int32_t)val1 < (int32_t)imm) ? 1 : 0; break; // SLTI
                case 3: res = (val1 < (uint32_t)imm) ? 1 : 0; break; // SLTIU
                case 4: res = val1 ^ imm; break; // XORI
                case 6: res = val1 | imm; break; // ORI
                case 7: res = val1 & imm; break; // ANDI
                case 1: res = val1 << (imm & 0x1F); break; // SLLI
                case 5: // SRLI / SRAI
                    if ((instr.raw >> 30) == 0) res = val1 >> (imm & 0x1F); // SRLI
                    else res = (int32_t)val1 >> (imm & 0x1F); // SRAI
                    break;
            }
            rf.write(instr.rd, res);
            break;
        }
        case 0x33: // R-type ALU
        {
            uint32_t val1 = rf.read(instr.rs1);
            uint32_t val2 = rf.read(instr.rs2);
            uint32_t res = 0;
            
            if (instr.funct7 == 0x01) { // RV32M Extension
                switch (instr.funct3) {
                    case 0: res = (uint32_t)((int32_t)val1 * (int32_t)val2); break; // MUL
                    case 1: res = (uint32_t)(((int64_t)(int32_t)val1 * (int64_t)(int32_t)val2) >> 32); break; // MULH
                    case 2: res = (uint32_t)(((int64_t)(int32_t)val1 * (uint64_t)val2) >> 32); break; // MULHSU
                    case 3: res = (uint32_t)(((uint64_t)val1 * (uint64_t)val2) >> 32); break; // MULHU
                    case 4: // DIV
                        if (val2 == 0) res = 0xFFFFFFFF;
                        else res = (int32_t)val1 / (int32_t)val2;
                        break;
                    case 5: // DIVU
                        if (val2 == 0) res = 0xFFFFFFFF;
                        else res = val1 / val2;
                        break;
                    case 6: // REM
                        if (val2 == 0) res = val1;
                        else res = (int32_t)val1 % (int32_t)val2;
                        break;
                    case 7: // REMU
                        if (val2 == 0) res = val1;
                        else res = val1 % val2;
                        break;
                }
            } else { // Standard RV32I
                switch (instr.funct3) {
                    case 0: 
                        if (instr.funct7 == 0x00) res = val1 + val2; // ADD
                        else res = val1 - val2; // SUB
                        break;
                    case 1: res = val1 << (val2 & 0x1F); break; // SLL
                    case 2: res = ((int32_t)val1 < (int32_t)val2) ? 1 : 0; break; // SLT
                    case 3: res = (val1 < val2) ? 1 : 0; break; // SLTU
                    case 4: res = val1 ^ val2; break; // XOR
                    case 5:
                        if (instr.funct7 == 0x00) res = val1 >> (val2 & 0x1F); // SRL
                        else res = (int32_t)val1 >> (val2 & 0x1F); // SRA
                        break;
                    case 6: res = val1 | val2; break; // OR
                    case 7: res = val1 & val2; break; // AND
                }
            }
            rf.write(instr.rd, res);
            break;
        }
    }

    pc = next_pc;
}

void Interpreter::dump_state() const {
    LOG_INFO("--- Execution State ---");
    LOG_INFO("PC: 0x" + Memory::to_hex_string(pc));
    LOG_INFO("Instructions executed: " + std::to_string(inst_count));
    rf.dump();
}

} // namespace phlego
