#include "cpu.h"

#include <array>
#include <cstdint>
#include <iomanip>
#include <iostream>

#include "logger.h"

constexpr uint32_t OPCODE_MASK = 0x7F;
constexpr uint32_t REGISTER_MASK = 0x1F;
constexpr uint32_t SIGN_EXTEND_MASK = 0xFFFFF000;

// #include <unordered_set>

// const std::unordered_set<ITypeFunct3> I_TYPE_ALU_INSTRUCTIONS = {
//     ITypeFunct3::ADDI, ITypeFunct3::SLTI, ITypeFunct3::SLTIU,
//     ITypeFunct3::XORI, ITypeFunct3::ORI, ITypeFunct3::ANDI,
//     ITypeFunct3::SLLI, ITypeFunct3::SRLI, ITypeFunct3::SRAI
// };

// if (std::holds_alternative<IType>(pipeline.execute.instruction)) {
//     const auto& instr = std::get<IType>(pipeline.execute.instruction);

//     if (I_TYPE_ALU_INSTRUCTIONS.count(instr.funct3)) {
//         // È una istruzione I-ALU
//     }
// }
// #include <unordered_set>

// const std::unordered_set<ITypeFunct3> I_TYPE_LOAD_INSTRUCTIONS = {
//     ITypeFunct3::LB, ITypeFunct3::LH, ITypeFunct3::LW,
//     ITypeFunct3::LBU, ITypeFunct3::LHU
// };

// if (std::holds_alternative<IType>(pipeline.execute.instruction)) {
//     const auto& instr = std::get<IType>(pipeline.execute.instruction);

//     if (I_TYPE_LOAD_INSTRUCTIONS.count(instr.funct3)) {
//         // È una istruzione I-LOAD
//     }
// }

// Constructor
/**
 * @brief Construct a new CPU object.
 *
 * @param memory Reference to the memory object.
 */
CPU::CPU(Memory &memory) : memory(memory), pc(0) {
    // Initialize registers to zero
    for (std::size_t i = 0; i < registers.size(); ++i) {
        registers[i] = {registerNames[i], 0};  // Inizializza i valori a 0
    }
}

/**
 * @brief Fetch the next instruction from memory.
 *
 * @param pipeline The pipeline state.
 */
void CPU::fetch() {
    if (!pipeline.stall && !pipeline.fetch.valid) {
        pipeline.fetch.instruction = memory.load_word(pc);
        pipeline.fetch.pc = pc;
        LOG_DEBUG("Fetched instruction: 0x" +
                  Memory::to_hex_string(pipeline.fetch.instruction) +
                  " from address: 0x" +
                  Memory::to_hex_string(pipeline.fetch.pc));
        pc += 4;
        pipeline.fetch.valid = true;
        pipeline.decode.pc = pipeline.fetch.pc;
    }
}

/**
 * @brief Decode the fetched instruction.
 *
 * @param pipeline The pipeline state.
 */
void CPU::decode() {
    if (!pipeline.stall && pipeline.fetch.valid) {
        uint32_t instruction = pipeline.fetch.instruction;
        if (instruction == 0) {
            LOG_ERROR("Encountered a zero instruction, which is unsupported.");
            throw std::runtime_error(
                "Unsupported instruction! Instruction: 0x0");
        }

        Opcode opcode = static_cast<Opcode>(instruction & OPCODE_MASK);
        LOG_DEBUG("Decoding instruction: " +
                  Memory::to_hex_string(instruction) + " with opcode: " +
                  Memory::to_hex_string(static_cast<uint8_t>(opcode)));

        switch (opcode) {
            case Opcode::R_TYPE: {
                RType r_type = {
                    static_cast<RTypeFunct3>((instruction >> 12) &
                                             0x7),  // funct3
                    static_cast<Funct7>((instruction >> 25) &
                                        OPCODE_MASK),  // funct7
                    static_cast<uint8_t>((instruction >> 7) &
                                         REGISTER_MASK),  // rd
                    static_cast<uint8_t>((instruction >> 15) &
                                         REGISTER_MASK),  // rs1
                    static_cast<uint8_t>((instruction >> 20) &
                                         REGISTER_MASK)  // rs2
                };
                LOG_DEBUG("Decoded R-Type: funct3=" +
                          std::to_string(static_cast<uint8_t>(r_type.funct3)) +
                          ", funct7=" +
                          std::to_string(static_cast<uint8_t>(r_type.funct7)) +
                          ", rd=" + std::to_string(r_type.rd) +
                          ", rs1=" + std::to_string(r_type.rs1) +
                          ", rs2=" + std::to_string(r_type.rs2));
                // Hazard detection
                // if (pipeline.execute.valid) {
                //     auto &exec_instr = pipeline.execute.instruction;
                //     if (std::holds_alternative<RType>(exec_instr)) {
                //         auto exec_r_type = std::get<RType>(exec_instr);
                //         if (exec_r_type.rd == r_type.rs1 ||
                //             exec_r_type.rd == r_type.rs2) {
                //             pipeline.stall = true;
                //             return;  // Stall the pipeline
                //         }
                //     }
                // }
                pipeline.decode.instruction = r_type;
                break;
            }
            case Opcode::I_TYPE_LOAD: {
                IType i_type = {
                    static_cast<ITypeFunct3>((instruction >> 12) &
                                             0x7),  // funct3
                    static_cast<uint8_t>((instruction >> 7) &
                                         REGISTER_MASK),  // rd
                    static_cast<uint8_t>((instruction >> 15) &
                                         REGISTER_MASK),     // rs1
                    static_cast<int32_t>(instruction) >> 20  // imm
                };
                LOG_DEBUG("Decoded I-Type Load: funct3=" +
                          std::to_string(static_cast<uint8_t>(i_type.funct3)) +
                          ", rd=" + std::to_string(i_type.rd) +
                          ", rs1=" + std::to_string(i_type.rs1) +
                          ", imm=" + std::to_string(i_type.imm));

                // Hazard detection
                // if (pipeline.execute.valid) {
                //     auto &exec_instr = pipeline.execute.instruction;
                //     if (std::holds_alternative<RType>(exec_instr)) {
                //         auto exec_r_type = std::get<RType>(exec_instr);
                //         if (exec_r_type.rd == i_type.rs1) {
                //             pipeline.stall = true;
                //             return;  // Stall the pipeline
                //         }
                //     }
                // }

                pipeline.decode.instruction = i_type;
                break;
            }
            case Opcode::I_TYPE_ALU: {
                IType i_type = {
                    static_cast<ITypeFunct3>((instruction >> 12) &
                                             0x7),  // funct3
                    static_cast<uint8_t>((instruction >> 7) &
                                         REGISTER_MASK),  // rd
                    static_cast<uint8_t>((instruction >> 15) &
                                         REGISTER_MASK),     // rs1
                    static_cast<int32_t>(instruction) >> 20  // imm
                };
                LOG_DEBUG("Decoded I-Type ALU: funct3=" +
                          std::to_string(static_cast<uint8_t>(i_type.funct3)) +
                          ", rd=" + std::to_string(i_type.rd) +
                          ", rs1=" + std::to_string(i_type.rs1) +
                          ", imm=" + std::to_string(i_type.imm));
                pipeline.decode.instruction = i_type;
                break;
            }
            case Opcode::JALR: {
                IType i_type = {
                    static_cast<ITypeFunct3>((instruction >> 12) &
                                             0x7),  // funct3
                    static_cast<uint8_t>((instruction >> 7) &
                                         REGISTER_MASK),  // rd
                    static_cast<uint8_t>((instruction >> 15) &
                                         REGISTER_MASK),     // rs1
                    static_cast<int32_t>(instruction) >> 20  // imm
                };
                LOG_DEBUG("Decoded JALR: funct3=" +
                          std::to_string(static_cast<uint8_t>(i_type.funct3)) +
                          ", rd=" + std::to_string(i_type.rd) +
                          ", rs1=" + std::to_string(i_type.rs1) +
                          ", imm=" + std::to_string(i_type.imm));
                pipeline.decode.instruction = i_type;
                break;
            }
            case Opcode::S_TYPE: {
                int32_t imm = ((instruction >> 7) & REGISTER_MASK) |
                              ((instruction >> 25) << 5);
                if (imm & 0x800)
                    imm |= SIGN_EXTEND_MASK;  // Sign-extend the immediate value
                SType s_type = {
                    static_cast<STypeFunct3>((instruction >> 12) &
                                             0x7),  // funct3
                    static_cast<uint8_t>((instruction >> 15) &
                                         REGISTER_MASK),  // rs1
                    static_cast<uint8_t>((instruction >> 20) &
                                         REGISTER_MASK),  // rs2
                    imm                                   // imm
                };
                LOG_DEBUG("Decoded S-Type: imm=" + std::to_string(s_type.imm) +
                          ", rs1=" + std::to_string(s_type.rs1) +
                          ", rs2=" + std::to_string(s_type.rs2) + ", funct3=" +
                          std::to_string(static_cast<uint8_t>(s_type.funct3)));
                pipeline.decode.instruction = s_type;
                break;
            }
            case Opcode::B_TYPE: {
                int32_t imm = ((instruction >> 7) & 0x1E) |
                              ((instruction >> 25) << 5) |
                              ((instruction & 0x80) << 4) |
                              ((instruction & 0x80000000) >> 19);
                if (imm & 0x1000)
                    imm |= 0xFFFFE000;  // Sign-extend the immediate value
                BType b_type = {
                    static_cast<BTypeFunct3>((instruction >> 12) &
                                             0x7),  // funct3
                    static_cast<uint8_t>((instruction >> 15) &
                                         REGISTER_MASK),  // rs1
                    static_cast<uint8_t>((instruction >> 20) &
                                         REGISTER_MASK),  // rs2
                    imm                                   // imm
                };
                LOG_DEBUG("Decoded B-Type: imm=" + std::to_string(b_type.imm) +
                          ", rs1=" + std::to_string(b_type.rs1) +
                          ", rs2=" + std::to_string(b_type.rs2) + ", funct3=" +
                          std::to_string(static_cast<uint8_t>(b_type.funct3)));
                pipeline.decode.instruction = b_type;
                break;
            }
            case Opcode::J_TYPE: {
                JType j_type = {
                    static_cast<uint8_t>((instruction >> 7) &
                                         REGISTER_MASK),  // rd
                    static_cast<int32_t>(
                        ((instruction >> 21) & 0x3FF) |       // imm[10:1]
                        ((instruction >> 20) & 0x1) << 11 |   // imm[11]
                        ((instruction >> 12) & 0xFF) << 12 |  // imm[19:12]
                        ((instruction & 0x80000000) ? 0xFFF00000
                                                    : 0)  // imm[31]
                        )};
                LOG_DEBUG("Decoded J-Type: rd=" + std::to_string(j_type.rd) +
                          ", imm=" + std::to_string(j_type.imm));
                pipeline.decode.instruction = j_type;
                break;
            }
            case Opcode::U_TYPE: {
                UType u_type = {
                    static_cast<uint8_t>((instruction >> 7) &
                                         REGISTER_MASK),                  // rd
                    static_cast<int32_t>(instruction & SIGN_EXTEND_MASK)  // imm
                };
                LOG_DEBUG("Decoded U-Type: rd=" + std::to_string(u_type.rd) +
                          ", imm=" + std::to_string(u_type.imm));
                pipeline.decode.instruction = u_type;
                break;
            }
            default:
                LOG_ERROR("Unsupported instruction! Instruction: 0x" +
                          Memory::to_hex_string(instruction));
                throw std::runtime_error(
                    "Unsupported instruction! Instruction: 0x" +
                    Memory::to_hex_string(instruction));
        }

        pipeline.execute.instruction = pipeline.decode.instruction;
        pipeline.execute.pc = pipeline.decode.pc;
        pipeline.fetch.valid = false;
        pipeline.decode.valid = true;
    }
}

/**
 * @brief Execute the decoded instruction.
 *
 * @param pipeline The pipeline state.
 */
void CPU::execute() {
    if (!pipeline.stall && pipeline.decode.valid) {
        pipeline.decode.valid = false;

        if (std::holds_alternative<RType>(pipeline.execute.instruction)) {
            auto r_type = std::get<RType>(pipeline.execute.instruction);

            // Forwarding logic for rs1
            if (pipeline.memory.valid &&
                std::holds_alternative<RType>(pipeline.memory.instruction)) {
                auto mem_r_type = std::get<RType>(pipeline.memory.instruction);
                if (mem_r_type.rd == r_type.rs1 && mem_r_type.rd != 0) {
                    registers[r_type.rs1].value = pipeline.memory.result;
                }
            } else if (pipeline.write_back.valid &&
                       pipeline.write_back.rd == r_type.rs1 &&
                       pipeline.write_back.rd != 0) {
                registers[r_type.rs1].value = pipeline.write_back.result;
            }

            // Forwarding logic for rs2
            if (pipeline.memory.valid &&
                std::holds_alternative<RType>(pipeline.memory.instruction)) {
                auto mem_r_type = std::get<RType>(pipeline.memory.instruction);
                if (mem_r_type.rd == r_type.rs2 && mem_r_type.rd != 0) {
                    registers[r_type.rs2].value = pipeline.memory.result;
                }
            } else if (pipeline.write_back.valid &&
                       pipeline.write_back.rd == r_type.rs2 &&
                       pipeline.write_back.rd != 0) {
                registers[r_type.rs2].value = pipeline.write_back.result;
            }

            pipeline.execute.alu_result = execute_r_type(r_type);
        } else if (std::holds_alternative<IType>(
                       pipeline.execute.instruction)) {
            auto i_type = std::get<IType>(pipeline.execute.instruction);
            if (static_cast<int>(i_type.funct3) == 0b000 && i_type.rd == 0 &&
                i_type.rs1 == 1 && i_type.imm == 0) {
                set_status_flag(StatusFlags::HALT);
                LOG_INFO("Encountered ret instruction. Halting execution.");
            } else if (i_type.funct3 == ITypeFunct3::ADDI ||
                       i_type.funct3 == ITypeFunct3::SLTI ||
                       i_type.funct3 == ITypeFunct3::SLTIU ||
                       i_type.funct3 == ITypeFunct3::XORI ||
                       i_type.funct3 == ITypeFunct3::ORI ||
                       i_type.funct3 == ITypeFunct3::ANDI ||
                       i_type.funct3 == ITypeFunct3::SLLI ||
                       i_type.funct3 == ITypeFunct3::SRLI ||
                       i_type.funct3 == ITypeFunct3::SRAI) {
                // Forwarding logic for rs1
                if (pipeline.memory.valid && std::holds_alternative<RType>(
                                                 pipeline.memory.instruction)) {
                    auto mem_r_type =
                        std::get<RType>(pipeline.memory.instruction);
                    if (mem_r_type.rd == i_type.rs1 && mem_r_type.rd != 0) {
                        registers[i_type.rs1].value = pipeline.memory.result;
                    }
                } else if (pipeline.write_back.valid &&
                           pipeline.write_back.rd == i_type.rs1 &&
                           pipeline.write_back.rd != 0) {
                    registers[i_type.rs1].value = pipeline.write_back.result;
                }
                pipeline.execute.alu_result = execute_i_type(i_type);
            } else if (i_type.funct3 == ITypeFunct3::LB ||
                       i_type.funct3 == ITypeFunct3::LH ||
                       i_type.funct3 == ITypeFunct3::LW ||
                       i_type.funct3 == ITypeFunct3::LBU ||
                       i_type.funct3 == ITypeFunct3::LHU) {
                int32_t sign_extended_imm = static_cast<int32_t>(i_type.imm);
                pipeline.execute.alu_result =
                    registers[i_type.rs1].value + sign_extended_imm;
            }
        } else if (std::holds_alternative<JType>(
                       pipeline.execute.instruction)) {
            auto j_type = std::get<JType>(pipeline.execute.instruction);
            execute_j_type(j_type);
        } else if (std::holds_alternative<SType>(
                       pipeline.execute.instruction)) {
            auto s_type = std::get<SType>(pipeline.execute.instruction);

            // Sign-extend the immediate value
            int32_t sign_extended_imm = static_cast<int32_t>(s_type.imm);

            // Perform the addition
            pipeline.execute.alu_result =
                registers[s_type.rs1].value + sign_extended_imm;
        } else if (std::holds_alternative<BType>(
                       pipeline.execute.instruction)) {
            auto b_type = std::get<BType>(pipeline.execute.instruction);
            execute_b_type(b_type);
        } else if (std::holds_alternative<UType>(
                       pipeline.execute.instruction)) {
            auto u_type = std::get<UType>(pipeline.execute.instruction);
            pipeline.execute.alu_result = execute_u_type(u_type);
        } else {
            LOG_ERROR("Unsupported instruction!");
            throw std::runtime_error("Unsupported instruction at PC: 0x" +
                                     Memory::to_hex_string(pc));
        }

        pipeline.memory.instruction = pipeline.execute.instruction;
        pipeline.memory.pc = pipeline.execute.pc;
        pipeline.execute.valid = true;
    }
}

/**
 * @brief Execute the memory stage.
 *
 * @param pipeline The pipeline state.
 */
void CPU::mem() {
    if (!pipeline.stall && pipeline.execute.valid) {
        pipeline.execute.valid = false;

        if (std::holds_alternative<IType>(pipeline.memory.instruction)) {
            auto i_type = std::get<IType>(pipeline.memory.instruction);
            if (i_type.funct3 == ITypeFunct3::LB ||
                i_type.funct3 == ITypeFunct3::LH ||
                i_type.funct3 == ITypeFunct3::LW ||
                i_type.funct3 == ITypeFunct3::LBU ||
                i_type.funct3 == ITypeFunct3::LHU) {
                uint32_t address = pipeline.execute.alu_result;
                LOG_DEBUG("Executing memory load at address: 0x" +
                          Memory::to_hex_string(address));
                switch (i_type.funct3) {
                    case ITypeFunct3::LB:
                        pipeline.memory.result =
                            (int8_t)memory.load_byte(address);
                        break;
                    case ITypeFunct3::LH:
                        pipeline.memory.result =
                            (int16_t)memory.load_half_word(address);
                        break;
                    case ITypeFunct3::LW:
                        pipeline.memory.result = memory.load_word(address);
                        break;
                    default:
                        LOG_ERROR("Unsupported load function! Funct3: " +
                                  std::to_string(
                                      static_cast<uint8_t>(i_type.funct3)));
                        std::cerr << "Unsupported load function! Funct3: "
                                  << static_cast<uint8_t>(i_type.funct3)
                                  << std::endl;
                }
                pipeline.write_back.rd = i_type.rd;
                pipeline.write_back.result = pipeline.memory.result;
            }
        } else if (std::holds_alternative<SType>(pipeline.memory.instruction)) {
            auto s_type = std::get<SType>(pipeline.memory.instruction);
            uint32_t address = pipeline.execute.alu_result;
            LOG_DEBUG("Executing memory store at address: 0x" +
                      Memory::to_hex_string(address));
            switch (s_type.funct3) {
                case STypeFunct3::SB:
                    memory.store_byte(address,
                                      registers[s_type.rs2].value & 0xFF);
                    break;
                case STypeFunct3::SH:
                    memory.store_half_word(
                        address, registers[s_type.rs2].value & 0xFFFF);
                    break;
                case STypeFunct3::SW:
                    memory.store_word(address, registers[s_type.rs2].value);
                    break;
                default:
                    LOG_ERROR(
                        "Unsupported store function! Funct3: " +
                        std::to_string(static_cast<uint8_t>(s_type.funct3)));
                    std::cerr << "Unsupported store function! Funct3: "
                              << static_cast<uint8_t>(s_type.funct3)
                              << std::endl;
            }
        }

        pipeline.write_back.instruction = pipeline.memory.instruction;
        pipeline.write_back.pc = pipeline.memory.pc;
        pipeline.memory.valid = true;
    }
}

/**
 * @brief Execute the write-back stage.
 *
 * @param pipeline The pipeline state.
 */
void CPU::write_back() {
    if (pipeline.memory.valid) {
        pipeline.memory.valid = false;

        if (std::holds_alternative<RType>(pipeline.write_back.instruction)) {
            auto r_type = std::get<RType>(pipeline.write_back.instruction);
            registers[r_type.rd].value = pipeline.execute.alu_result;
            LOG_DEBUG("Write-back R-Type: x" + std::to_string(r_type.rd) +
                      " = " + std::to_string(pipeline.execute.alu_result));
        } else if (std::holds_alternative<IType>(
                       pipeline.write_back.instruction)) {
            auto i_type = std::get<IType>(pipeline.write_back.instruction);
            if (i_type.funct3 == ITypeFunct3::ADDI ||
                i_type.funct3 == ITypeFunct3::SLTI ||
                i_type.funct3 == ITypeFunct3::SLTIU ||
                i_type.funct3 == ITypeFunct3::XORI ||
                i_type.funct3 == ITypeFunct3::ORI ||
                i_type.funct3 == ITypeFunct3::ANDI ||
                i_type.funct3 == ITypeFunct3::SLLI ||
                i_type.funct3 == ITypeFunct3::SRLI ||
                i_type.funct3 == ITypeFunct3::SRAI) {
                registers[i_type.rd].value = pipeline.execute.alu_result;
                LOG_DEBUG("Write-back I-Type: x" + std::to_string(i_type.rd) +
                          " = " +
                          Memory::to_hex_string(pipeline.execute.alu_result));
            } else if (i_type.funct3 == ITypeFunct3::LB ||
                       i_type.funct3 == ITypeFunct3::LH ||
                       i_type.funct3 == ITypeFunct3::LW ||
                       i_type.funct3 == ITypeFunct3::LBU ||
                       i_type.funct3 == ITypeFunct3::LHU) {
                registers[i_type.rd].value = pipeline.memory.result;
                LOG_DEBUG("Write-back I-Type: x" + std::to_string(i_type.rd) +
                          " = " +
                          Memory::to_hex_string(pipeline.memory.result));
            }
        } else if (std::holds_alternative<UType>(
                       pipeline.write_back.instruction)) {
            auto u_type = std::get<UType>(pipeline.write_back.instruction);
            registers[u_type.rd].value = pipeline.execute.alu_result;
            LOG_DEBUG("Write-back U-Type: x" + std::to_string(u_type.rd) +
                      " = " + std::to_string(pipeline.execute.alu_result));
        }
    }
}

/**
 * @brief Execute an I-Type instruction.
 *
 * @param pipeline The pipeline state.
 * @param instr The decoded I-Type instruction.
 */
uint32_t CPU::execute_i_type(const IType &instr) {
    uint32_t result = 0;
    LOG_DEBUG("Executing I-Type instruction");
    switch (instr.funct3) {
        case ITypeFunct3::ADDI:
            result = registers[instr.rs1].value + instr.imm;
            LOG_DEBUG("Executed ADDI: x" + std::to_string(instr.rd) + " = x" +
                      std::to_string(instr.rs1) + " + " +
                      std::to_string(instr.imm));
            break;
        case ITypeFunct3::SLLI:
            result = registers[instr.rs1].value << (instr.imm & REGISTER_MASK);
            LOG_DEBUG("Executed SLLI: x" + std::to_string(instr.rd) + " = x" +
                      std::to_string(instr.rs1) + " << " +
                      std::to_string(instr.imm & REGISTER_MASK));
            break;
        case ITypeFunct3::SLTI:
            result = (int32_t)registers[instr.rs1].value < (int32_t)instr.imm
                         ? 1
                         : 0;
            LOG_DEBUG("Executed SLTI: x" + std::to_string(instr.rd) + " = x" +
                      std::to_string(instr.rs1) + " < " +
                      std::to_string(instr.imm));
            break;
        case ITypeFunct3::SLTIU:
            result = registers[instr.rs1].value < (uint32_t)instr.imm ? 1 : 0;
            LOG_DEBUG("Executed SLTIU: x" + std::to_string(instr.rd) + " = x" +
                      std::to_string(instr.rs1) + " < " +
                      std::to_string(instr.imm));
            break;
        case ITypeFunct3::XORI:
            result = registers[instr.rs1].value ^ instr.imm;
            LOG_DEBUG("Executed XORI: x" + std::to_string(instr.rd) + " = x" +
                      std::to_string(instr.rs1) + " ^ " +
                      std::to_string(instr.imm));
            break;
        case ITypeFunct3::SRLI:
            // case ITypeFunct3::SRAI:
            if ((instr.imm & 0x40000000) == 0) {
                result =
                    registers[instr.rs1].value >> (instr.imm & REGISTER_MASK);
                LOG_DEBUG("Executed SRLI: x" + std::to_string(instr.rd) +
                          " = x" + std::to_string(instr.rs1) + " >> " +
                          std::to_string(instr.imm & REGISTER_MASK));
            } else {
                result = (int32_t)registers[instr.rs1].value >>
                         (instr.imm & REGISTER_MASK);
                LOG_DEBUG("Executed SRAI: x" + std::to_string(instr.rd) +
                          " = (int32_t)x" + std::to_string(instr.rs1) + " >> " +
                          std::to_string(instr.imm & REGISTER_MASK));
            }
            break;
        case ITypeFunct3::ORI:
            result = registers[instr.rs1].value | instr.imm;
            LOG_DEBUG("Executed ORI: x" + std::to_string(instr.rd) + " = x" +
                      std::to_string(instr.rs1) + " | " +
                      std::to_string(instr.imm));
            break;
        case ITypeFunct3::ANDI:
            result = registers[instr.rs1].value & instr.imm;
            LOG_DEBUG("Executed ANDI: x" + std::to_string(instr.rd) + " = x" +
                      std::to_string(instr.rs1) + " & " +
                      std::to_string(instr.imm));
            break;
        default:
            LOG_ERROR("Unsupported I-Type function! Funct3: " +
                      std::to_string(static_cast<uint8_t>(instr.funct3)));
            std::cerr << "Unsupported I-Type function! Funct3: "
                      << static_cast<uint8_t>(instr.funct3) << std::endl;
    }
    return result;
}

/**
 * @brief Execute a store instruction.
 *
 * @param instr The decoded S-Type instruction.
 */
void CPU::execute_s_type(const SType &instr) {
    uint32_t address = registers[instr.rs1].value + instr.imm;
    LOG_DEBUG("Executing store instruction at address: 0x" +
              Memory::to_hex_string(address));
    switch (instr.funct3) {
        case STypeFunct3::SB:
            memory.store_byte(address, registers[instr.rs2].value & 0xFF);
            LOG_DEBUG("Stored byte from register x" +
                      std::to_string(instr.rs2) + " to address: 0x" +
                      Memory::to_hex_string(address));
            break;
        case STypeFunct3::SH:
            memory.store_half_word(address,
                                   registers[instr.rs2].value & 0xFFFF);
            LOG_DEBUG("Stored half word from register x" +
                      std::to_string(instr.rs2) + " to address: 0x" +
                      Memory::to_hex_string(address));
            break;
        case STypeFunct3::SW:
            memory.store_word(address, registers[instr.rs2].value);
            LOG_DEBUG("Stored word from register x" +
                      std::to_string(instr.rs2) + " to address: 0x" +
                      Memory::to_hex_string(address));
            break;
        default:
            LOG_ERROR("Unsupported store function! Funct3: " +
                      std::to_string(static_cast<uint8_t>(instr.funct3)));
            std::cerr << "Unsupported store function! Funct3: "
                      << static_cast<uint8_t>(instr.funct3) << std::endl;
    }
}

/**
 * @brief Execute an R-Type instruction.
 *
 * @param instr The decoded R-Type instruction.
 */
uint32_t CPU::execute_r_type(const RType &instr) {
    LOG_DEBUG("Executing R-Type instruction");

    uint32_t result = 0;
    switch (instr.funct3) {
        case RTypeFunct3::ADD:
            // case RTypeFunct3::SUB:
            // case RTypeFunct3::MUL:
            if (instr.funct7 == Funct7::ADD) {  // ADD
                result =
                    registers[instr.rs1].value + registers[instr.rs2].value;
                LOG_DEBUG("Executed ADD: x" + std::to_string(instr.rd) +
                          " = x" + std::to_string(instr.rs1) + " + x" +
                          std::to_string(instr.rs2));
            } else if (instr.funct7 == Funct7::SUB) {  // SUB
                result =
                    registers[instr.rs1].value - registers[instr.rs2].value;
                LOG_DEBUG("Executed SUB: x" + std::to_string(instr.rd) +
                          " = x" + std::to_string(instr.rs1) + " - x" +
                          std::to_string(instr.rs2));
            } else if (instr.funct7 == Funct7::MUL) {  // MUL
                result =
                    registers[instr.rs1].value * registers[instr.rs2].value;
                LOG_DEBUG("Executed MUL: x" + std::to_string(instr.rd) +
                          " = x" + std::to_string(instr.rs1) + " * x" +
                          std::to_string(instr.rs2));
            }
            break;
        case RTypeFunct3::SLL:
            // case RTypeFunct3::MULH:
            if (instr.funct7 == Funct7::SLL) {
                result = registers[instr.rs1].value
                         << (registers[instr.rs2].value & REGISTER_MASK);
                LOG_DEBUG(
                    "Executed SLL: x" + std::to_string(instr.rd) + " = x" +
                    std::to_string(instr.rs1) + " << " +
                    std::to_string(registers[instr.rs2].value & REGISTER_MASK));
            } else if (instr.funct7 == Funct7::MULH) {
                int64_t result_mul = (int64_t)registers[instr.rs1].value *
                                     (int64_t)registers[instr.rs2].value;
                result = result_mul >> 32;
                LOG_DEBUG("Executed MULH: x" + std::to_string(instr.rd) +
                          " = (int64_t)x" + std::to_string(instr.rs1) +
                          " * (int64_t)x" + std::to_string(instr.rs2) +
                          " >> 32");
            }
            break;
        case RTypeFunct3::SLT:
            // case RTypeFunct3::MULHSU:
            if (instr.funct7 == Funct7::SLT) {
                result = (int32_t)registers[instr.rs1].value <
                                 (int32_t)registers[instr.rs2].value
                             ? 1
                             : 0;
                LOG_DEBUG("Executed SLT: x" + std::to_string(instr.rd) +
                          " = x" + std::to_string(instr.rs1) + " < x" +
                          std::to_string(instr.rs2));
            } else if (instr.funct7 == Funct7::MULHSU) {
                int64_t result_mul = (int64_t)registers[instr.rs1].value *
                                     (uint64_t)registers[instr.rs2].value;
                result = result_mul >> 32;
                LOG_DEBUG("Executed MULHSU: x" + std::to_string(instr.rd) +
                          " = (int64_t)x" + std::to_string(instr.rs1) +
                          " * (uint64_t)x" + std::to_string(instr.rs2) +
                          " >> 32");
            }
            break;
        case RTypeFunct3::SLTU:
            // case RTypeFunct3::MULHU:
            if (instr.funct7 == Funct7::SLTU) {
                result = registers[instr.rs1].value < registers[instr.rs2].value
                             ? 1
                             : 0;
                LOG_DEBUG("Executed SLTU: x" + std::to_string(instr.rd) +
                          " = x" + std::to_string(instr.rs1) + " < x" +
                          std::to_string(instr.rs2));
            } else if (instr.funct7 == Funct7::MULHU) {
                uint64_t result_mul = (uint64_t)registers[instr.rs1].value *
                                      (uint64_t)registers[instr.rs2].value;
                result = result_mul >> 32;
                LOG_DEBUG("Executed MULHU: x" + std::to_string(instr.rd) +
                          " = (uint64_t)x" + std::to_string(instr.rs1) +
                          " * (uint64_t)x" + std::to_string(instr.rs2) +
                          " >> 32");
            }
            break;
        case RTypeFunct3::XOR:
            // case RTypeFunct3::DIV:
            if (instr.funct7 == Funct7::XOR) {
                result =
                    registers[instr.rs1].value ^ registers[instr.rs2].value;
                LOG_DEBUG("Executed XOR: x" + std::to_string(instr.rd) +
                          " = x" + std::to_string(instr.rs1) + " ^ x" +
                          std::to_string(instr.rs2));
            } else if (instr.funct7 == Funct7::DIV) {
                if (registers[instr.rs2].value == 0) {
                    LOG_ERROR("Division by zero!");
                    set_status_flag(StatusFlags::DIV_ZERO);
                    throw std::runtime_error(
                        "Division by zero in instruction at PC: 0x" +
                        Memory::to_hex_string(pc));
                }
                result = (int32_t)registers[instr.rs1].value /
                         (int32_t)registers[instr.rs2].value;
                LOG_DEBUG("Executed DIV: x" + std::to_string(instr.rd) +
                          " = x" + std::to_string(instr.rs1) + " / x" +
                          std::to_string(instr.rs2));
            }
            break;
        case RTypeFunct3::SRL:
            // case RTypeFunct3::SRA:
            if (instr.funct7 == Funct7::SRL) {  // SRL
                result = registers[instr.rs1].value >>
                         (registers[instr.rs2].value & REGISTER_MASK);
                LOG_DEBUG(
                    "Executed SRL: x" + std::to_string(instr.rd) + " = x" +
                    std::to_string(instr.rs1) + " >> " +
                    std::to_string(registers[instr.rs2].value & REGISTER_MASK));
            } else if (instr.funct7 == Funct7::SRA) {  // SRA
                result = (int32_t)registers[instr.rs1].value >>
                         (registers[instr.rs2].value & REGISTER_MASK);
                LOG_DEBUG(
                    "Executed SRA: x" + std::to_string(instr.rd) +
                    " = (int32_t)x" + std::to_string(instr.rs1) + " >> " +
                    std::to_string(registers[instr.rs2].value & REGISTER_MASK));
            }
            break;
        case RTypeFunct3::OR:
            // case RTypeFunct3::REM:
            if (instr.funct7 == Funct7::OR) {
                result =
                    registers[instr.rs1].value | registers[instr.rs2].value;
                LOG_DEBUG("Executed OR: x" + std::to_string(instr.rd) + " = x" +
                          std::to_string(instr.rs1) + " | x" +
                          std::to_string(instr.rs2));
            } else if (instr.funct7 == Funct7::REM) {
                if (registers[instr.rs2].value == 0) {
                    LOG_ERROR("Remainder by zero!");
                    throw std::runtime_error("Remainder by zero!");
                }
                result = (int32_t)registers[instr.rs1].value %
                         (int32_t)registers[instr.rs2].value;
                LOG_DEBUG("Executed REM: x" + std::to_string(instr.rd) +
                          " = x" + std::to_string(instr.rs1) + " % x" +
                          std::to_string(instr.rs2));
            }
            break;
        case RTypeFunct3::AND:
            // case RTypeFunct3::REMU:
            if (instr.funct7 == Funct7::AND) {
                result =
                    registers[instr.rs1].value & registers[instr.rs2].value;
                LOG_DEBUG("Executed AND: x" + std::to_string(instr.rd) +
                          " = x" + std::to_string(instr.rs1) + " & x" +
                          std::to_string(instr.rs2));
            } else if (instr.funct7 == Funct7::REMU) {
                if (registers[instr.rs2].value == 0) {
                    LOG_ERROR("Remainder by zero!");
                    throw std::runtime_error("Remainder by zero!");
                }
                result =
                    registers[instr.rs1].value % registers[instr.rs2].value;
                LOG_DEBUG("Executed REMU: x" + std::to_string(instr.rd) +
                          " = x" + std::to_string(instr.rs1) + " % x" +
                          std::to_string(instr.rs2));
            }
            break;
        default:
            LOG_ERROR("Unsupported funct3 for R-Type");
            throw std::invalid_argument("Unsupported funct3 for R-Type");
    }
    return result;
}

/**
 * @brief Execute a B-Type instruction.
 *
 * @param instr The decoded B-Type instruction.
 */
void CPU::execute_b_type(const BType &instr) {
    uint32_t target = pc + instr.imm;
    LOG_DEBUG("Executing B-Type instruction with target address: 0x" +
              Memory::to_hex_string(target));

    switch (instr.funct3) {
        case BTypeFunct3::BEQ:
            if (registers[instr.rs1].value == registers[instr.rs2].value) {
                pc = target - 4;  // Adjust PC since it's already incremented
                LOG_DEBUG("Executed BEQ: Branch taken to address: 0x" +
                          Memory::to_hex_string(pc));
            }
            break;
        case BTypeFunct3::BNE:
            if (registers[instr.rs1].value != registers[instr.rs2].value) {
                pc = target - 4;
                LOG_DEBUG("Executed BNE: Branch taken to address: 0x" +
                          Memory::to_hex_string(pc));
            }
            break;
        default:
            LOG_ERROR("Unsupported B-Type function! Funct3: " +
                      std::to_string(static_cast<uint8_t>(instr.funct3)));
            std::cerr << "Unsupported B-Type function! Funct3: "
                      << static_cast<uint8_t>(instr.funct3) << std::endl;
    }
}

/**
 * @brief Execute a J-Type instruction.
 *
 * @param instr The decoded J-Type instruction.
 */
void CPU::execute_j_type(const JType &instr) {
    LOG_DEBUG("Executing J-Type instruction");
    registers[instr.rd].value = pc + 4;
    pc += instr.imm - 4;
    LOG_DEBUG("Executed JAL: x" + std::to_string(instr.rd) + " = 0x" +
              Memory::to_hex_string(pc));
}

/**
 * @brief Execute a U-Type instruction.
 *
 * @param instr The decoded U-Type instruction.
 * @return uint32_t The result of the ALU operation.
 */
uint32_t CPU::execute_u_type(const UType &instr) {
    LOG_DEBUG("Executing U-Type instruction");
    uint32_t result = instr.imm;
    LOG_DEBUG("Executed LUI: x" + std::to_string(instr.rd) + " = " +
              std::to_string(instr.imm));
    return result;
}

/**
 * @brief Set the program counter.
 *
 * @param address The address to set the program counter to.
 */
void CPU::set_pc(uint32_t address) {
    pc = address;
    LOG_DEBUG("Program counter set to: 0x" + Memory::to_hex_string(pc));
}

/**
 * @brief Set the stack pointer.
 *
 * @param address The address to set the stack pointer to.
 */
void CPU::set_sp(uint32_t address) {
    registers[2].value = address;
    LOG_DEBUG("Stack pointer set to: 0x" +
              Memory::to_hex_string(registers[2].value));
}

/**
 * @brief Check for data hazards and set stall signals.
 *
 * @param pipeline The pipeline state.
 * @return true if a stall is needed, false otherwise.
 */
// bool CPU::detect_hazard() {
//     if (pipeline.decode.valid) {
//         auto &decoded = pipeline.decode.instruction;
//         uint32_t rs1 = 0, rs2 = 0;

//         if (std::holds_alternative<RType>(decoded)) {
//             auto r_type = std::get<RType>(decoded);
//             rs1 = r_type.rs1;
//             rs2 = r_type.rs2;
//         } else if (std::holds_alternative<IType>(decoded)) {
//             auto i_type = std::get<IType>(decoded);
//             rs1 = i_type.rs1;
//         }

//         if (pipeline.execute.valid) {
//             auto &exec_instr = pipeline.execute.instruction;
//             if (std::holds_alternative<RType>(exec_instr)) {
//                 auto exec_r_type = std::get<RType>(exec_instr);
//                 if (exec_r_type.rd == rs1 || exec_r_type.rd == rs2) {
//                     return true;  // Hazard detected
//                 }
//             } else if (std::holds_alternative<IType>(exec_instr)) {
//                 auto exec_i_type = std::get<IType>(exec_instr);
//                 if (exec_i_type.rd == rs1) {
//                     return true;  // Hazard detected
//                 }
//             }
//         }

//         if (pipeline.memory.valid) {
//             auto &mem_instr = pipeline.memory.instruction;
//             if (std::holds_alternative<RType>(mem_instr)) {
//                 auto mem_r_type = std::get<RType>(mem_instr);
//                 if (mem_r_type.rd == rs1 || mem_r_type.rd == rs2) {
//                     return true;  // Hazard detected
//                 }
//             } else if (std::holds_alternative<IType>(mem_instr)) {
//                 auto mem_i_type = std::get<IType>(mem_instr);
//                 if (mem_i_type.rd == rs1) {
//                     return true;  // Hazard detected
//                 }
//             }
//         }
//     }

//     return false;  // No hazard detected
// }

/**
 * @brief Print the CPU registers.
 */
void CPU::print_registers() const {
    std::cout << "PC: 0x" << Memory::to_hex_string(pc) << std::endl;
    for (size_t i = 0; i < 32; ++i) {
        std::cout << std::left << std::setw(4) << registers[i].name << ": "
                  << std::right << std::setw(8) << std::setfill(' ')
                  << Memory::to_hex_string(registers[i].value) << std::endl;
    }
    std::cout << std::endl;
}

/**
 * @brief Get the program counter.
 *
 * @return uint32_t The program counter.
 */
uint32_t CPU::get_pc() const { return pc; }

/**
 * @brief Get the text size.
 *
 * @return uint32_t The text size.
 */
uint32_t CPU::get_text_size() const {
    return memory.get_memory_layout().text_size;
}

bool CPU::can_fetch() { return !pipeline.fetch.valid; }

bool CPU::can_decode() { return pipeline.fetch.valid; }

bool CPU::can_execute() { return pipeline.decode.valid; }

bool CPU::can_mem() { return pipeline.execute.valid; }

bool CPU::can_write_back() { return pipeline.memory.valid; }

uint32_t CPU::get_status() const { return status; }

void CPU::set_status(uint32_t status) {
    this->status = status;
    LOG_DEBUG("Status register set to: 0x" + Memory::to_hex_string(status));
}

void CPU::set_status_flag(StatusFlags flag) {
    status |= static_cast<uint32_t>(flag);
    LOG_DEBUG("Status flag set: 0x" +
              Memory::to_hex_string(static_cast<uint32_t>(flag)));
}

void CPU::clear_status_flag(StatusFlags flag) {
    status &= ~static_cast<uint32_t>(flag);
    LOG_DEBUG("Status flag cleared: 0x" +
              Memory::to_hex_string(static_cast<uint32_t>(flag)));
}

bool CPU::is_halted() const {
    return status & static_cast<uint32_t>(StatusFlags::HALT);
}