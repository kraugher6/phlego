#include "cpu.h"
#include "logger.h"
#include <fstream>

// Constructor
CPU::CPU(Memory& memory) : memory(memory), pc(0) {
    // Initialize registers to zero
    for (auto& reg : registers) {
        reg = 0;
    }

    // Initialize stack pointer with the address read from the ELF file
    registers[2] = memory.get_stack_pointer();

    LOG_INFO("CPU initialized with stack pointer set to: 0x" + Memory::to_hex_string(registers[2]));
}

std::variant<RType, IType, SType, BType, JType> CPU::decode(uint32_t instruction) {
    if (instruction == 0) {
        LOG_ERROR("Encountered a zero instruction, which is unsupported.");
        throw std::runtime_error("Unsupported instruction! Instruction: 0x0");
    }

    Opcode opcode = static_cast<Opcode>(instruction & 0x7F);
    LOG_DEBUG("Decoding instruction: 0x" + Memory::to_hex_string(instruction) + " with opcode: 0x" + Memory::to_hex_string(static_cast<uint8_t>(opcode)));

    switch (opcode) {
        case Opcode::R_TYPE: {
            RType r_type = {
                static_cast<uint8_t>((instruction >> 12) & 0x7),          // funct3
                static_cast<uint8_t>((instruction >> 25) & 0x7F),         // funct7
                static_cast<uint8_t>((instruction >> 7) & 0x1F),          // rd
                static_cast<uint8_t>((instruction >> 15) & 0x1F),         // rs1
                static_cast<uint8_t>((instruction >> 20) & 0x1F)          // rs2
            };
            LOG_DEBUG("Decoded R-Type: funct3=" + std::to_string(r_type.funct3) +
                      ", funct7=" + std::to_string(r_type.funct7) +
                      ", rd=" + std::to_string(r_type.rd) +
                      ", rs1=" + std::to_string(r_type.rs1) +
                      ", rs2=" + std::to_string(r_type.rs2));
            return r_type;
        }
        case Opcode::I_TYPE_LOAD: {
            IType i_type = {
                static_cast<uint8_t>((instruction >> 12) & 0x7),          // funct3
                static_cast<uint8_t>((instruction >> 7) & 0x1F),          // rd
                static_cast<uint8_t>((instruction >> 15) & 0x1F),         // rs1
                static_cast<int32_t>(instruction) >> 20                   // imm
            };
            LOG_DEBUG("Decoded I-Type Load: funct3=" + std::to_string(i_type.funct3) +
                      ", rd=" + std::to_string(i_type.rd) +
                      ", rs1=" + std::to_string(i_type.rs1) +
                      ", imm=" + std::to_string(i_type.imm));
            return i_type;
        }
        case Opcode::I_TYPE_ALU: {
            IType i_type = {
                static_cast<uint8_t>((instruction >> 12) & 0x7),          // funct3
                static_cast<uint8_t>((instruction >> 7) & 0x1F),          // rd
                static_cast<uint8_t>((instruction >> 15) & 0x1F),         // rs1
                static_cast<int32_t>(instruction) >> 20                   // imm
            };
            LOG_DEBUG("Decoded I-Type ALU: funct3=" + std::to_string(i_type.funct3) +
                      ", rd=" + std::to_string(i_type.rd) +
                      ", rs1=" + std::to_string(i_type.rs1) +
                      ", imm=" + std::to_string(i_type.imm));
            return i_type;
        }
        case Opcode::JALR: {
            IType i_type = {
                static_cast<uint8_t>((instruction >> 12) & 0x7),          // funct3
                static_cast<uint8_t>((instruction >> 7) & 0x1F),          // rd
                static_cast<uint8_t>((instruction >> 15) & 0x1F),         // rs1
                static_cast<int32_t>(instruction) >> 20                   // imm
            };
            LOG_DEBUG("Decoded JALR: funct3=" + std::to_string(i_type.funct3) +
                      ", rd=" + std::to_string(i_type.rd) +
                      ", rs1=" + std::to_string(i_type.rs1) +
                      ", imm=" + std::to_string(i_type.imm));
            return i_type;
        }
        case Opcode::S_TYPE: {
            int32_t imm = ((instruction >> 7) & 0x1F) | ((instruction >> 25) << 5);
            if (imm & 0x800) imm |= 0xFFFFF000; // Sign-extend the immediate value
            SType s_type = {
                imm, // imm
                static_cast<uint8_t>((instruction >> 15) & 0x1F),         // rs1
                static_cast<uint8_t>((instruction >> 20) & 0x1F),         // rs2
                static_cast<uint8_t>((instruction >> 12) & 0x7)           // funct3
            };
            LOG_DEBUG("Decoded S-Type: imm=" + std::to_string(s_type.imm) +
                      ", rs1=" + std::to_string(s_type.rs1) +
                      ", rs2=" + std::to_string(s_type.rs2) +
                      ", funct3=" + std::to_string(s_type.funct3));
            return s_type;
        }
        case Opcode::B_TYPE: {
            int32_t imm = ((instruction >> 7) & 0x1E) | ((instruction >> 25) << 5) | ((instruction & 0x80) << 4) | ((instruction & 0x80000000) >> 19);
            if (imm & 0x1000) imm |= 0xFFFFE000; // Sign-extend the immediate value
            BType b_type = {
                imm, // imm
                static_cast<uint8_t>((instruction >> 15) & 0x1F),         // rs1
                static_cast<uint8_t>((instruction >> 20) & 0x1F),         // rs2
                static_cast<uint8_t>((instruction >> 12) & 0x7)           // funct3
            };
            LOG_DEBUG("Decoded B-Type: imm=" + std::to_string(b_type.imm) +
                      ", rs1=" + std::to_string(b_type.rs1) +
                      ", rs2=" + std::to_string(b_type.rs2) +
                      ", funct3=" + std::to_string(b_type.funct3));
            return b_type;
        }
        case Opcode::J_TYPE: {
            JType j_type = {
                static_cast<uint8_t>((instruction >> 7) & 0x1F),          // rd
                static_cast<int32_t>(
                    ((instruction >> 21) & 0x3FF) |     // imm[10:1]
                    ((instruction >> 20) & 0x1) << 11 | // imm[11]
                    ((instruction >> 12) & 0xFF) << 12 |// imm[19:12]
                    ((instruction & 0x80000000) ? 0xFFF00000 : 0) // imm[31]
                )
            };
            LOG_DEBUG("Decoded J-Type: rd=" + std::to_string(j_type.rd) +
                      ", imm=" + std::to_string(j_type.imm));
            return j_type;
        }
        default:
            LOG_ERROR("Unsupported instruction! Instruction: 0x" + Memory::to_hex_string(instruction));
            throw std::runtime_error("Unsupported instruction! Instruction: 0x" + Memory::to_hex_string(instruction));
    }
}

void CPU::run() {
    // Dump CPU registers before starting execution
    LOG_INFO("CPU state at start:");
    print_registers();

    while (true) {
        // fetch the instruction
        uint32_t instruction = memory.load_word(pc);
        LOG_DEBUG("Fetched instruction: 0x" + Memory::to_hex_string(instruction) + " from address: 0x" + Memory::to_hex_string(pc));

        // decode the instruction
        auto decoded = decode(instruction);

        // execute the instruction
        if (std::holds_alternative<RType>(decoded)) {
            auto r_type = std::get<RType>(decoded);
            execute_r_type(r_type);
        } else if (std::holds_alternative<IType>(decoded)) {
            auto i_type = std::get<IType>(decoded);
            if (static_cast<Opcode>(instruction & 0x7F) == Opcode::I_TYPE_LOAD) {
                execute_load(i_type);
            } else {
                execute_alu(i_type);
            }
        } else if (std::holds_alternative<JType>(decoded)) {
            auto j_type = std::get<JType>(decoded);
            execute_j_type(j_type);
        } else if (std::holds_alternative<SType>(decoded)) {
            auto s_type = std::get<SType>(decoded);
            execute_store(s_type);
        } else if (std::holds_alternative<BType>(decoded)) {
            auto b_type = std::get<BType>(decoded);
            execute_b_type(b_type);
        } else {
            LOG_ERROR("Unsupported instruction!");
            throw std::runtime_error("Unsupported instruction!");
        }

        // Check for the ret instruction (0x00008067)
        if (instruction == 0x00008067) {
            LOG_INFO("Encountered ret instruction. Terminating execution.");
            break;
        }

        // Dump CPU registers after executing the instruction
        LOG_INFO("CPU state after execution:");
        print_registers();

        pc += 4; // Advance the program counter
    }
}

void CPU::execute_load(const IType& instr) {
    int32_t sign_extended_imm = static_cast<int32_t>(instr.imm);
    uint32_t address = registers[instr.rs1] + sign_extended_imm;
    LOG_DEBUG("Executing load instruction at address: 0x" + Memory::to_hex_string(address));
    switch (static_cast<Funct3>(instr.funct3)) {
        case Funct3::LB:
            registers[instr.rd] = (int8_t)memory.load_byte(address);
            LOG_DEBUG("Loaded byte from address: 0x" + Memory::to_hex_string(address) + " into register x" + std::to_string(instr.rd));
            break;
        case Funct3::LH:
            registers[instr.rd] = (int16_t)memory.load_half_word(address);
            LOG_DEBUG("Loaded half word from address: 0x" + Memory::to_hex_string(address) + " into register x" + std::to_string(instr.rd));
            break;
        case Funct3::LW:
            registers[instr.rd] = memory.load_word(address);
            LOG_DEBUG("Loaded word from address: 0x" + Memory::to_hex_string(address) + " into register x" + std::to_string(instr.rd));
            break;
        default:
            LOG_ERROR("Unsupported load function! Funct3: " + std::to_string(instr.funct3));
            std::cerr << "Unsupported load function! Funct3: " << +instr.funct3 << std::endl;
    }
}

void CPU::execute_alu(const IType& instr) {
    LOG_DEBUG("Executing ALU instruction");
    switch (static_cast<Funct3>(instr.funct3)) {
        case Funct3::ADD_SUB:
            registers[instr.rd] = registers[instr.rs1] + instr.imm;
            LOG_DEBUG("Executed ADDI: x" + std::to_string(instr.rd) + " = x" + std::to_string(instr.rs1) + " + " + std::to_string(instr.imm));
            break;
        default:
            LOG_ERROR("Unsupported ALU function! Funct3: " + std::to_string(instr.funct3));
            std::cerr << "Unsupported ALU function! Funct3: " << +instr.funct3 << std::endl;
    }
}

void CPU::execute_store(const SType& instr) {
    uint32_t address = registers[instr.rs1] + instr.imm;
    LOG_DEBUG("Executing store instruction at address: 0x" + Memory::to_hex_string(address));
    switch (static_cast<Funct3>(instr.funct3)) {
        case Funct3::SB:
            memory.store_byte(address, registers[instr.rs2] & 0xFF);
            LOG_DEBUG("Stored byte from register x" + std::to_string(instr.rs2) + " to address: 0x" + Memory::to_hex_string(address));
            break;
        case Funct3::SH:
            memory.store_half_word(address, registers[instr.rs2] & 0xFFFF);
            LOG_DEBUG("Stored half word from register x" + std::to_string(instr.rs2) + " to address: 0x" + Memory::to_hex_string(address));
            break;
        case Funct3::SW:
            memory.store_word(address, registers[instr.rs2]);
            LOG_DEBUG("Stored word from register x" + std::to_string(instr.rs2) + " to address: 0x" + Memory::to_hex_string(address));
            break;
        default:
            LOG_ERROR("Unsupported store function! Funct3: " + std::to_string(instr.funct3));
            std::cerr << "Unsupported store function! Funct3: " << +instr.funct3 << std::endl;
    }
}

void CPU::execute_r_type(const RType& instr) {
    LOG_DEBUG("Executing R-Type instruction");
    switch (static_cast<Funct3>(instr.funct3)) {
        case Funct3::ADD_SUB:
            if (instr.funct7 == 0x00) { // ADD
                registers[instr.rd] = registers[instr.rs1] + registers[instr.rs2];
                LOG_DEBUG("Executed ADD: x" + std::to_string(instr.rd) + " = x" + std::to_string(instr.rs1) + " + x" + std::to_string(instr.rs2));
            } else if (instr.funct7 == 0x20) { // SUB
                registers[instr.rd] = registers[instr.rs1] - registers[instr.rs2];
                LOG_DEBUG("Executed SUB: x" + std::to_string(instr.rd) + " = x" + std::to_string(instr.rs1) + " - x" + std::to_string(instr.rs2));
            }
            break;
        default:
            LOG_ERROR("Unsupported R-Type function!");
            std::cerr << "Unsupported R-Type function!" << std::endl;
    }
}

void CPU::execute_b_type(const BType& instr) {
    uint32_t target = pc + instr.imm;
    LOG_DEBUG("Executing B-Type instruction with target address: 0x" + Memory::to_hex_string(target));

    switch (static_cast<Funct3>(instr.funct3)) {
        case Funct3::BEQ:
            if (registers[instr.rs1] == registers[instr.rs2]) {
                pc = target - 4; // Adjust PC since it's already incremented
                LOG_DEBUG("Executed BEQ: Branch taken to address: 0x" + Memory::to_hex_string(pc));
            }
            break;
        case Funct3::BNE:
            if (registers[instr.rs1] != registers[instr.rs2]) {
                pc = target - 4;
                LOG_DEBUG("Executed BNE: Branch taken to address: 0x" + Memory::to_hex_string(pc));
            }
            break;
        default:
            LOG_ERROR("Unsupported B-Type function! Funct3: " + std::to_string(instr.funct3));
            std::cerr << "Unsupported B-Type function! Funct3: " << +instr.funct3 << std::endl;
    }
}

void CPU::execute_j_type(const JType& instr) {
    LOG_DEBUG("Executing J-Type instruction");
    registers[instr.rd] = pc + 4;
    pc += instr.imm - 4;
    LOG_DEBUG("Executed JAL: x" + std::to_string(instr.rd) + " = 0x" + Memory::to_hex_string(pc));
}

void CPU::set_pc(uint32_t address) {
    pc = address;
    LOG_DEBUG("Program counter set to: 0x" + Memory::to_hex_string(pc));
}

void CPU::set_sp(uint32_t address) {
    registers[2] = address;
    LOG_DEBUG("Stack pointer set to: 0x" + Memory::to_hex_string(registers[2]));
}

void CPU::print_registers() const {
    std::cout << "PC: 0x" << std::hex << pc;
    for (int i = 0; i < 32; ++i) {
        std::cout << " x" << i << ": 0x" << std::hex << registers[i];
    }
    std::cout << std::endl;
}
