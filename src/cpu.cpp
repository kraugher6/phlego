#include "cpu.h"
#include "logger.h"
#include <fstream>
#include <iostream>
#include <iomanip>
#include <array>

// Constructor
/**
 * @brief Construct a new CPU object.
 *
 * @param memory Reference to the memory object.
 */
CPU::CPU(Memory &memory) : memory(memory), pc(0)
{
    // Initialize registers to zero
    for (std::size_t i = 0; i < registers.size(); ++i)
    {
        registers[i] = {registerNames[i], 0}; // Inizializza i valori a 0
    }
}

/**
 * @brief Fetch the next instruction from memory.
 *
 * @param pipeline The pipeline state.
 */
void CPU::fetch()
{
    std::unique_lock<std::mutex> lock(fetch_mutex);
    fetch_cv.wait(lock, [this]
                  { return !pipeline.stall && !pipeline.fetch.valid; });
    if (!pipeline.stall && !pipeline.fetch.valid)
    {
        pipeline.fetch.instruction = memory.load_word(pc);
        pipeline.fetch.pc = pc;
        pc += 4;
        pipeline.fetch.valid = true;
        LOG_DEBUG("Fetched instruction: 0x" + Memory::to_hex_string(pipeline.fetch.instruction) + " from address: 0x" + Memory::to_hex_string(pc));
    }
    decode_cv.notify_all();
}

/**
 * @brief Decode the fetched instruction.
 *
 * @param pipeline The pipeline state.
 */
void CPU::decode()
{
    std::unique_lock<std::mutex> lock(decode_mutex);
    decode_cv.wait(lock, [this]
                   { return pipeline.fetch.valid; });

    if (!pipeline.stall && pipeline.fetch.valid)
    {
        uint32_t instruction = pipeline.fetch.instruction;
        if (instruction == 0)
        {
            LOG_ERROR("Encountered a zero instruction, which is unsupported.");
            throw std::runtime_error("Unsupported instruction! Instruction: 0x0");
        }

        Opcode opcode = static_cast<Opcode>(instruction & 0x7F);
        LOG_DEBUG("Decoding instruction: 0x" + Memory::to_hex_string(instruction) + " with opcode: 0x" + Memory::to_hex_string(static_cast<uint8_t>(opcode)));

        switch (opcode)
        {
        case Opcode::R_TYPE:
        {
            RType r_type = {
                static_cast<RTypeFunct3>((instruction >> 12) & 0x7), // funct3
                static_cast<Funct7>((instruction >> 25) & 0x7F),     // funct7
                static_cast<uint8_t>((instruction >> 7) & 0x1F),     // rd
                static_cast<uint8_t>((instruction >> 15) & 0x1F),    // rs1
                static_cast<uint8_t>((instruction >> 20) & 0x1F)     // rs2
            };
            LOG_DEBUG("Decoded R-Type: funct3=" + std::to_string(static_cast<uint8_t>(r_type.funct3)) +
                      ", funct7=" + std::to_string(static_cast<uint8_t>(r_type.funct7)) +
                      ", rd=" + std::to_string(r_type.rd) +
                      ", rs1=" + std::to_string(r_type.rs1) +
                      ", rs2=" + std::to_string(r_type.rs2));
            pipeline.decode.decoded_instruction = r_type;
            break;
        }
        case Opcode::I_TYPE_LOAD:
        {
            IType i_type = {
                static_cast<ITypeFunct3>((instruction >> 12) & 0x7), // funct3
                static_cast<uint8_t>((instruction >> 7) & 0x1F),     // rd
                static_cast<uint8_t>((instruction >> 15) & 0x1F),    // rs1
                static_cast<int32_t>(instruction) >> 20              // imm
            };
            LOG_DEBUG("Decoded I-Type Load: funct3=" + std::to_string(static_cast<uint8_t>(i_type.funct3)) +
                      ", rd=" + std::to_string(i_type.rd) +
                      ", rs1=" + std::to_string(i_type.rs1) +
                      ", imm=" + std::to_string(i_type.imm));
            pipeline.decode.decoded_instruction = i_type;
            break;
        }
        case Opcode::I_TYPE_ALU:
        {
            IType i_type = {
                static_cast<ITypeFunct3>((instruction >> 12) & 0x7), // funct3
                static_cast<uint8_t>((instruction >> 7) & 0x1F),     // rd
                static_cast<uint8_t>((instruction >> 15) & 0x1F),    // rs1
                static_cast<int32_t>(instruction) >> 20              // imm
            };
            LOG_DEBUG("Decoded I-Type ALU: funct3=" + std::to_string(static_cast<uint8_t>(i_type.funct3)) +
                      ", rd=" + std::to_string(i_type.rd) +
                      ", rs1=" + std::to_string(i_type.rs1) +
                      ", imm=" + std::to_string(i_type.imm));
            pipeline.decode.decoded_instruction = i_type;
            break;
        }
        case Opcode::JALR:
        {
            IType i_type = {
                static_cast<ITypeFunct3>((instruction >> 12) & 0x7), // funct3
                static_cast<uint8_t>((instruction >> 7) & 0x1F),     // rd
                static_cast<uint8_t>((instruction >> 15) & 0x1F),    // rs1
                static_cast<int32_t>(instruction) >> 20              // imm
            };
            LOG_DEBUG("Decoded JALR: funct3=" + std::to_string(static_cast<uint8_t>(i_type.funct3)) +
                      ", rd=" + std::to_string(i_type.rd) +
                      ", rs1=" + std::to_string(i_type.rs1) +
                      ", imm=" + std::to_string(i_type.imm));
            pipeline.decode.decoded_instruction = i_type;
            break;
        }
        case Opcode::S_TYPE:
        {
            int32_t imm = ((instruction >> 7) & 0x1F) | ((instruction >> 25) << 5);
            if (imm & 0x800)
                imm |= 0xFFFFF000; // Sign-extend the immediate value
            SType s_type = {
                static_cast<STypeFunct3>((instruction >> 12) & 0x7), // funct3
                static_cast<uint8_t>((instruction >> 15) & 0x1F),    // rs1
                static_cast<uint8_t>((instruction >> 20) & 0x1F),    // rs2
                imm                                                  // imm
            };
            LOG_DEBUG("Decoded S-Type: imm=" + std::to_string(s_type.imm) +
                      ", rs1=" + std::to_string(s_type.rs1) +
                      ", rs2=" + std::to_string(s_type.rs2) +
                      ", funct3=" + std::to_string(static_cast<uint8_t>(s_type.funct3)));
            pipeline.decode.decoded_instruction = s_type;
            break;
        }
        case Opcode::B_TYPE:
        {
            int32_t imm = ((instruction >> 7) & 0x1E) | ((instruction >> 25) << 5) | ((instruction & 0x80) << 4) | ((instruction & 0x80000000) >> 19);
            if (imm & 0x1000)
                imm |= 0xFFFFE000; // Sign-extend the immediate value
            BType b_type = {
                static_cast<BTypeFunct3>((instruction >> 12) & 0x7), // funct3
                static_cast<uint8_t>((instruction >> 15) & 0x1F),    // rs1
                static_cast<uint8_t>((instruction >> 20) & 0x1F),    // rs2
                imm                                                  // imm
            };
            LOG_DEBUG("Decoded B-Type: imm=" + std::to_string(b_type.imm) +
                      ", rs1=" + std::to_string(b_type.rs1) +
                      ", rs2=" + std::to_string(b_type.rs2) +
                      ", funct3=" + std::to_string(static_cast<uint8_t>(b_type.funct3)));
            pipeline.decode.decoded_instruction = b_type;
            break;
        }
        case Opcode::J_TYPE:
        {
            JType j_type = {
                static_cast<uint8_t>((instruction >> 7) & 0x1F), // rd
                static_cast<int32_t>(
                    ((instruction >> 21) & 0x3FF) |               // imm[10:1]
                    ((instruction >> 20) & 0x1) << 11 |           // imm[11]
                    ((instruction >> 12) & 0xFF) << 12 |          // imm[19:12]
                    ((instruction & 0x80000000) ? 0xFFF00000 : 0) // imm[31]
                    )};
            LOG_DEBUG("Decoded J-Type: rd=" + std::to_string(j_type.rd) +
                      ", imm=" + std::to_string(j_type.imm));
            pipeline.decode.decoded_instruction = j_type;
            break;
        }
        case Opcode::U_TYPE:
        {
            UType u_type = {
                static_cast<uint8_t>((instruction >> 7) & 0x1F), // rd
                static_cast<int32_t>(instruction & 0xFFFFF000)   // imm
            };
            LOG_DEBUG("Decoded U-Type: rd=" + std::to_string(u_type.rd) +
                      ", imm=" + std::to_string(u_type.imm));
            pipeline.decode.decoded_instruction = u_type;
            break;
        }
        default:
            LOG_ERROR("Unsupported instruction! Instruction: 0x" + Memory::to_hex_string(instruction));
            throw std::runtime_error("Unsupported instruction! Instruction: 0x" + Memory::to_hex_string(instruction));
        }

        pipeline.decode.pc = pipeline.fetch.pc;
        pipeline.decode.valid = true;
        pipeline.fetch.valid = false;
        LOG_DEBUG("Decoded instruction at address: 0x" + Memory::to_hex_string(pipeline.decode.pc));
    }
    execute_cv.notify_all();
}

/**
 * @brief Execute the decoded instruction.
 *
 * @param pipeline The pipeline state.
 */
void CPU::execute()
{
    std::unique_lock<std::mutex> lock(execute_mutex);
    execute_cv.wait(lock, [this]
                    { return pipeline.decode.valid; });

    if (!pipeline.stall && pipeline.decode.valid)
    {
        auto &decoded = pipeline.decode.decoded_instruction;
        pipeline.execute.instruction = decoded;
        pipeline.execute.pc = pipeline.decode.pc;
        pipeline.execute.valid = true;
        pipeline.decode.valid = false;

        if (std::holds_alternative<RType>(decoded))
        {
            auto r_type = std::get<RType>(decoded);
            // Forwarding logic
            // if (pipeline.memory.valid && std::holds_alternative<RType>(pipeline.memory.instruction)) {
            //     auto mem_r_type = std::get<RType>(pipeline.memory.instruction);
            //     if (mem_r_type.rd == r_type.rs1) {
            //         rs1_value = pipeline.memory.result;
            //     }
            //     if (mem_r_type.rd == r_type.rs2) {
            //         rs2_value = pipeline.memory.result;
            //     }
            // }
            pipeline.execute.alu_result = execute_r_type(r_type);
        }
        else if (std::holds_alternative<IType>(decoded))
        {
            auto i_type = std::get<IType>(decoded);
            if (static_cast<Opcode>(pipeline.fetch.instruction & 0x7F) == Opcode::I_TYPE_ALU)
            {
                // Forwarding logic
                // if (pipeline.memory.valid && std::holds_alternative<RType>(pipeline.memory.instruction)) {
                //     auto mem_r_type = std::get<RType>(pipeline.memory.instruction);
                //     if (mem_r_type.rd == i_type.rs1) {
                //         rs1_value = pipeline.memory.result;
                //     }
                // }
                pipeline.execute.alu_result = execute_i_type(i_type);
            }
            else
            {
                int32_t sign_extended_imm = static_cast<int32_t>(i_type.imm);
                pipeline.execute.alu_result = registers[i_type.rs1].value + sign_extended_imm;
            }
        }
        else if (std::holds_alternative<JType>(decoded))
        {
            auto j_type = std::get<JType>(decoded);
            execute_j_type(j_type);
        }
        else if (std::holds_alternative<SType>(decoded))
        {
            auto s_type = std::get<SType>(decoded);

            // Sign-extend the immediate value
            int32_t sign_extended_imm = static_cast<int32_t>(s_type.imm);

            // Perform the addition
            pipeline.execute.alu_result = registers[s_type.rs1].value + sign_extended_imm;
        }
        else if (std::holds_alternative<BType>(decoded))
        {
            auto b_type = std::get<BType>(decoded);
            execute_b_type(b_type);
        }
        else if (std::holds_alternative<UType>(decoded))
        {
            auto u_type = std::get<UType>(decoded);
            pipeline.execute.alu_result = execute_u_type(u_type);
        }
        else
        {
            LOG_ERROR("Unsupported instruction!");
            throw std::runtime_error("Unsupported instruction!");
        }
        mem_cv.notify_all();
    }
}

/**
 * @brief Execute an I-Type instruction.
 *
 * @param pipeline The pipeline state.
 * @param instr The decoded I-Type instruction.
 */
uint32_t CPU::execute_i_type(const IType &instr)
{
    uint32_t result = 0;
    LOG_DEBUG("Executing I-Type instruction");
    switch (instr.funct3)
    {
    case ITypeFunct3::ADDI:
        result = registers[instr.rs1].value + instr.imm;
        LOG_DEBUG("Executed ADDI: x" + std::to_string(instr.rd) + " = x" + std::to_string(instr.rs1) + " + " + std::to_string(instr.imm));
        break;
    case ITypeFunct3::SLLI:
        result = registers[instr.rs1].value << (instr.imm & 0x1F);
        LOG_DEBUG("Executed SLLI: x" + std::to_string(instr.rd) + " = x" + std::to_string(instr.rs1) + " << " + std::to_string(instr.imm & 0x1F));
        break;
    case ITypeFunct3::SLTI:
        result = (int32_t)registers[instr.rs1].value < (int32_t)instr.imm ? 1 : 0;
        LOG_DEBUG("Executed SLTI: x" + std::to_string(instr.rd) + " = x" + std::to_string(instr.rs1) + " < " + std::to_string(instr.imm));
        break;
    case ITypeFunct3::SLTIU:
        result = registers[instr.rs1].value < (uint32_t)instr.imm ? 1 : 0;
        LOG_DEBUG("Executed SLTIU: x" + std::to_string(instr.rd) + " = x" + std::to_string(instr.rs1) + " < " + std::to_string(instr.imm));
        break;
    case ITypeFunct3::XORI:
        result = registers[instr.rs1].value ^ instr.imm;
        LOG_DEBUG("Executed XORI: x" + std::to_string(instr.rd) + " = x" + std::to_string(instr.rs1) + " ^ " + std::to_string(instr.imm));
        break;
    case ITypeFunct3::SRLI:
        // case ITypeFunct3::SRAI:
        if ((instr.imm & 0x40000000) == 0)
        {
            result = registers[instr.rs1].value >> (instr.imm & 0x1F);
            LOG_DEBUG("Executed SRLI: x" + std::to_string(instr.rd) + " = x" + std::to_string(instr.rs1) + " >> " + std::to_string(instr.imm & 0x1F));
        }
        else
        {
            result = (int32_t)registers[instr.rs1].value >> (instr.imm & 0x1F);
            LOG_DEBUG("Executed SRAI: x" + std::to_string(instr.rd) + " = (int32_t)x" + std::to_string(instr.rs1) + " >> " + std::to_string(instr.imm & 0x1F));
        }
        break;
    case ITypeFunct3::ORI:
        result = registers[instr.rs1].value | instr.imm;
        LOG_DEBUG("Executed ORI: x" + std::to_string(instr.rd) + " = x" + std::to_string(instr.rs1) + " | " + std::to_string(instr.imm));
        break;
    case ITypeFunct3::ANDI:
        result = registers[instr.rs1].value & instr.imm;
        LOG_DEBUG("Executed ANDI: x" + std::to_string(instr.rd) + " = x" + std::to_string(instr.rs1) + " & " + std::to_string(instr.imm));
        break;
    default:
        LOG_ERROR("Unsupported I-Type function! Funct3: " + std::to_string(static_cast<uint8_t>(instr.funct3)));
        std::cerr << "Unsupported I-Type function! Funct3: " << static_cast<uint8_t>(instr.funct3) << std::endl;
    }
    return result;
}

/**
 * @brief Execute the memory stage.
 *
 * @param pipeline The pipeline state.
 */
void CPU::mem()
{
    std::unique_lock<std::mutex> lock(mem_mutex);
    mem_cv.wait(lock, [this]
                { return pipeline.execute.valid; });

    if (!pipeline.stall && pipeline.execute.valid)
    {
        auto &instr = pipeline.execute.instruction;
        pipeline.memory.instruction = instr;
        pipeline.memory.pc = pipeline.execute.pc;
        pipeline.memory.valid = true;
        pipeline.execute.valid = false;

        if (std::holds_alternative<IType>(instr))
        {
            auto i_type = std::get<IType>(instr);
            if (static_cast<Opcode>(pipeline.fetch.instruction & 0x7F) == Opcode::I_TYPE_LOAD)
            {
                uint32_t address = pipeline.execute.alu_result;
                LOG_DEBUG("Executing memory load at address: 0x" + Memory::to_hex_string(address));
                switch (i_type.funct3)
                {
                case ITypeFunct3::LB:
                    pipeline.memory.result = (int8_t)memory.load_byte(address);
                    break;
                case ITypeFunct3::LH:
                    pipeline.memory.result = (int16_t)memory.load_half_word(address);
                    break;
                case ITypeFunct3::LW:
                    pipeline.memory.result = memory.load_word(address);
                    break;
                default:
                    LOG_ERROR("Unsupported load function! Funct3: " + std::to_string(static_cast<uint8_t>(i_type.funct3)));
                    std::cerr << "Unsupported load function! Funct3: " << static_cast<uint8_t>(i_type.funct3) << std::endl;
                }
                pipeline.write_back = {pipeline.memory.pc, i_type.rd, pipeline.memory.result, true};
            }
        }
        else if (std::holds_alternative<SType>(instr))
        {
            auto s_type = std::get<SType>(instr);
            uint32_t address = pipeline.execute.alu_result;
            LOG_DEBUG("Executing memory store at address: 0x" + Memory::to_hex_string(address));
            switch (s_type.funct3)
            {
            case STypeFunct3::SB:
                memory.store_byte(address, registers[s_type.rs2].value & 0xFF);
                break;
            case STypeFunct3::SH:
                memory.store_half_word(address, registers[s_type.rs2].value & 0xFFFF);
                break;
            case STypeFunct3::SW:
                memory.store_word(address, registers[s_type.rs2].value);
                break;
            default:
                LOG_ERROR("Unsupported store function! Funct3: " + std::to_string(static_cast<uint8_t>(s_type.funct3)));
                std::cerr << "Unsupported store function! Funct3: " << static_cast<uint8_t>(s_type.funct3) << std::endl;
            }
        }
        write_back_cv.notify_all();
    }
}

/**
 * @brief Execute the fetch-decode-run cycle.
 */
void CPU::run()
{
    // Dump CPU registers before starting execution
    LOG_INFO("CPU state at start:");
    print_registers();

    // while (true)
    // {
    //     // Check for hazards and set stall signal
    //     pipeline.stall = detect_hazard();

    //     if (!pipeline.stall)
    //     {
    //         // Fetch stage
    //         fetch();

    //         // Decode stage
    //         decode();

    //         // Execute stage
    //         execute();

    //         // Memory stage
    //         mem();

    //         // Write-back stage
    //         write_back();
    //     }
    //     else
    //     {
    //         LOG_WARN("Stall detected. Pipeline stages are stalled.");
    //     }

    //     // Check for the ret instruction (0x00008067)
    //     if (pipeline.fetch.instruction == 0x00008067)
    //     {
    //         LOG_INFO("Encountered ret instruction. Terminating execution.");
    //         break;
    //     }

    //     // Dump CPU registers after executing the instruction
    //     LOG_INFO("CPU state after execution:");
    //     print_registers();
    // }

    start_threads();

    // Wait for the threads to finish
    if (pipeline.fetch.instruction == 0x00008067)
    {
        stop_threads();
        LOG_INFO("Encountered ret instruction. Terminating execution.");
    }

    // Dump CPU registers after executing the instruction
    LOG_INFO("CPU state after execution:");
    print_registers();
}

/**
 * @brief Execute a store instruction.
 *
 * @param instr The decoded S-Type instruction.
 */
void CPU::execute_s_type(const SType &instr)
{
    uint32_t address = registers[instr.rs1].value + instr.imm;
    LOG_DEBUG("Executing store instruction at address: 0x" + Memory::to_hex_string(address));
    switch (instr.funct3)
    {
    case STypeFunct3::SB:
        memory.store_byte(address, registers[instr.rs2].value & 0xFF);
        LOG_DEBUG("Stored byte from register x" + std::to_string(instr.rs2) + " to address: 0x" + Memory::to_hex_string(address));
        break;
    case STypeFunct3::SH:
        memory.store_half_word(address, registers[instr.rs2].value & 0xFFFF);
        LOG_DEBUG("Stored half word from register x" + std::to_string(instr.rs2) + " to address: 0x" + Memory::to_hex_string(address));
        break;
    case STypeFunct3::SW:
        memory.store_word(address, registers[instr.rs2].value);
        LOG_DEBUG("Stored word from register x" + std::to_string(instr.rs2) + " to address: 0x" + Memory::to_hex_string(address));
        break;
    default:
        LOG_ERROR("Unsupported store function! Funct3: " + std::to_string(static_cast<uint8_t>(instr.funct3)));
        std::cerr << "Unsupported store function! Funct3: " << static_cast<uint8_t>(instr.funct3) << std::endl;
    }
}

/**
 * @brief Execute an R-Type instruction.
 *
 * @param instr The decoded R-Type instruction.
 */
uint32_t CPU::execute_r_type(const RType &instr)
{
    LOG_DEBUG("Executing R-Type instruction");

    uint32_t result = 0;
    switch (instr.funct3)
    {
    case RTypeFunct3::ADD:
        // case RTypeFunct3::SUB:
        // case RTypeFunct3::MUL:
        if (instr.funct7 == Funct7::ADD)
        { // ADD
            result = registers[instr.rs1].value + registers[instr.rs2].value;
            LOG_DEBUG("Executed ADD: x" + std::to_string(instr.rd) + " = x" + std::to_string(instr.rs1) + " + x" + std::to_string(instr.rs2));
        }
        else if (instr.funct7 == Funct7::SUB)
        { // SUB
            result = registers[instr.rs1].value - registers[instr.rs2].value;
            LOG_DEBUG("Executed SUB: x" + std::to_string(instr.rd) + " = x" + std::to_string(instr.rs1) + " - x" + std::to_string(instr.rs2));
        }
        else if (instr.funct7 == Funct7::MUL)
        { // MUL
            result = registers[instr.rs1].value * registers[instr.rs2].value;
            LOG_DEBUG("Executed MUL: x" + std::to_string(instr.rd) + " = x" + std::to_string(instr.rs1) + " * x" + std::to_string(instr.rs2));
        }
        break;
    case RTypeFunct3::SLL:
        // case RTypeFunct3::MULH:
        if (instr.funct7 == Funct7::SLL)
        {
            result = registers[instr.rs1].value << (registers[instr.rs2].value & 0x1F);
            LOG_DEBUG("Executed SLL: x" + std::to_string(instr.rd) + " = x" + std::to_string(instr.rs1) + " << " + std::to_string(registers[instr.rs2] & 0x1F));
        }
        else if (instr.funct7 == Funct7::MULH)
        {
            int64_t result_mul = (int64_t)registers[instr.rs1].value * (int64_t)registers[instr.rs2].value;
            result = result_mul >> 32;
            LOG_DEBUG("Executed MULH: x" + std::to_string(instr.rd) + " = (int64_t)x" + std::to_string(instr.rs1) + " * (int64_t)x" + std::to_string(instr.rs2) + " >> 32");
        }
        break;
    case RTypeFunct3::SLT:
        // case RTypeFunct3::MULHSU:
        if (instr.funct7 == Funct7::SLT)
        {
            result = (int32_t)registers[instr.rs1].value < (int32_t)registers[instr.rs2].value ? 1 : 0;
            LOG_DEBUG("Executed SLT: x" + std::to_string(instr.rd) + " = x" + std::to_string(instr.rs1) + " < x" + std::to_string(instr.rs2));
        }
        else if (instr.funct7 == Funct7::MULHSU)
        {
            int64_t result_mul = (int64_t)registers[instr.rs1].value * (uint64_t)registers[instr.rs2].value;
            result = result_mul >> 32;
            LOG_DEBUG("Executed MULHSU: x" + std::to_string(instr.rd) + " = (int64_t)x" + std::to_string(instr.rs1) + " * (uint64_t)x" + std::to_string(instr.rs2) + " >> 32");
        }
        break;
    case RTypeFunct3::SLTU:
        // case RTypeFunct3::MULHU:
        if (instr.funct7 == Funct7::SLTU)
        {
            result = registers[instr.rs1].value < registers[instr.rs2].value ? 1 : 0;
            LOG_DEBUG("Executed SLTU: x" + std::to_string(instr.rd) + " = x" + std::to_string(instr.rs1) + " < x" + std::to_string(instr.rs2));
        }
        else if (instr.funct7 == Funct7::MULHU)
        {
            uint64_t result_mul = (uint64_t)registers[instr.rs1].value * (uint64_t)registers[instr.rs2].value;
            result = result_mul >> 32;
            LOG_DEBUG("Executed MULHU: x" + std::to_string(instr.rd) + " = (uint64_t)x" + std::to_string(instr.rs1) + " * (uint64_t)x" + std::to_string(instr.rs2) + " >> 32");
        }
        break;
    case RTypeFunct3::XOR:
        // case RTypeFunct3::DIV:
        if (instr.funct7 == Funct7::XOR)
        {
            result = registers[instr.rs1].value ^ registers[instr.rs2].value;
            LOG_DEBUG("Executed XOR: x" + std::to_string(instr.rd) + " = x" + std::to_string(instr.rs1) + " ^ x" + std::to_string(instr.rs2));
        }
        else if (instr.funct7 == Funct7::DIV)
        {
            if (registers[instr.rs2].value == 0)
            {
                LOG_ERROR("Division by zero!");
                throw std::runtime_error("Division by zero!");
            }
            result = (int32_t)registers[instr.rs1].value / (int32_t)registers[instr.rs2].value;
            LOG_DEBUG("Executed DIV: x" + std::to_string(instr.rd) + " = x" + std::to_string(instr.rs1) + " / x" + std::to_string(instr.rs2));
        }
        break;
    case RTypeFunct3::SRL:
        // case RTypeFunct3::SRA:
        if (instr.funct7 == Funct7::SRL)
        { // SRL
            result = registers[instr.rs1].value >> (registers[instr.rs2].value & 0x1F);
            LOG_DEBUG("Executed SRL: x" + std::to_string(instr.rd) + " = x" + std::to_string(instr.rs1) + " >> " + std::to_string(registers[instr.rs2].value & 0x1F));
        }
        else if (instr.funct7 == Funct7::SRA)
        { // SRA
            result = (int32_t)registers[instr.rs1].value >> (registers[instr.rs2].value & 0x1F);
            LOG_DEBUG("Executed SRA: x" + std::to_string(instr.rd) + " = (int32_t)x" + std::to_string(instr.rs1) + " >> " + std::to_string(registers[instr.rs2].value & 0x1F));
        }
        break;
    case RTypeFunct3::OR:
        // case RTypeFunct3::REM:
        if (instr.funct7 == Funct7::OR)
        {
            result = registers[instr.rs1].value | registers[instr.rs2].value;
            LOG_DEBUG("Executed OR: x" + std::to_string(instr.rd) + " = x" + std::to_string(instr.rs1) + " | x" + std::to_string(instr.rs2));
        }
        else if (instr.funct7 == Funct7::REM)
        {
            if (registers[instr.rs2].value == 0)
            {
                LOG_ERROR("Remainder by zero!");
                throw std::runtime_error("Remainder by zero!");
            }
            result = (int32_t)registers[instr.rs1].value % (int32_t)registers[instr.rs2].value;
            LOG_DEBUG("Executed REM: x" + std::to_string(instr.rd) + " = x" + std::to_string(instr.rs1) + " % x" + std::to_string(instr.rs2));
        }
        break;
    case RTypeFunct3::AND:
        // case RTypeFunct3::REMU:
        if (instr.funct7 == Funct7::AND)
        {
            result = registers[instr.rs1].value & registers[instr.rs2].value;
            LOG_DEBUG("Executed AND: x" + std::to_string(instr.rd) + " = x" + std::to_string(instr.rs1) + " & x" + std::to_string(instr.rs2));
        }
        else if (instr.funct7 == Funct7::REMU)
        {
            if (registers[instr.rs2].value == 0)
            {
                LOG_ERROR("Remainder by zero!");
                throw std::runtime_error("Remainder by zero!");
            }
            result = registers[instr.rs1].value % registers[instr.rs2].value;
            LOG_DEBUG("Executed REMU: x" + std::to_string(instr.rd) + " = x" + std::to_string(instr.rs1) + " % x" + std::to_string(instr.rs2));
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
void CPU::execute_b_type(const BType &instr)
{
    uint32_t target = pc + instr.imm;
    LOG_DEBUG("Executing B-Type instruction with target address: 0x" + Memory::to_hex_string(target));

    switch (instr.funct3)
    {
    case BTypeFunct3::BEQ:
        if (registers[instr.rs1].value == registers[instr.rs2].value)
        {
            pc = target - 4; // Adjust PC since it's already incremented
            LOG_DEBUG("Executed BEQ: Branch taken to address: 0x" + Memory::to_hex_string(pc));
        }
        break;
    case BTypeFunct3::BNE:
        if (registers[instr.rs1].value != registers[instr.rs2].value)
        {
            pc = target - 4;
            LOG_DEBUG("Executed BNE: Branch taken to address: 0x" + Memory::to_hex_string(pc));
        }
        break;
    default:
        LOG_ERROR("Unsupported B-Type function! Funct3: " + std::to_string(static_cast<uint8_t>(instr.funct3)));
        std::cerr << "Unsupported B-Type function! Funct3: " << static_cast<uint8_t>(instr.funct3) << std::endl;
    }
}

/**
 * @brief Execute a J-Type instruction.
 *
 * @param instr The decoded J-Type instruction.
 */
void CPU::execute_j_type(const JType &instr)
{
    LOG_DEBUG("Executing J-Type instruction");
    registers[instr.rd].value = pc + 4;
    pc += instr.imm - 4;
    LOG_DEBUG("Executed JAL: x" + std::to_string(instr.rd) + " = 0x" + Memory::to_hex_string(pc));
}

/**
 * @brief Execute a U-Type instruction.
 *
 * @param instr The decoded U-Type instruction.
 * @return uint32_t The result of the ALU operation.
 */
uint32_t CPU::execute_u_type(const UType &instr)
{
    LOG_DEBUG("Executing U-Type instruction");
    uint32_t result = instr.imm;
    LOG_DEBUG("Executed LUI: x" + std::to_string(instr.rd) + " = " + std::to_string(instr.imm));
    return result;
}

/**
 * @brief Set the program counter.
 *
 * @param address The address to set the program counter to.
 */
void CPU::set_pc(uint32_t address)
{
    pc = address;
    LOG_DEBUG("Program counter set to: 0x" + Memory::to_hex_string(pc));
}

/**
 * @brief Set the stack pointer.
 *
 * @param address The address to set the stack pointer to.
 */
void CPU::set_sp(uint32_t address)
{
    registers[2].value = address;
    LOG_DEBUG("Stack pointer set to: 0x" + Memory::to_hex_string(registers[2].value));
}

/**
 * @brief Check for data hazards and set stall signals.
 *
 * @param pipeline The pipeline state.
 * @return true if a stall is needed, false otherwise.
 */
bool CPU::detect_hazard()
{
    if (pipeline.decode.valid)
    {
        auto &decoded = pipeline.decode.decoded_instruction;
        uint32_t rs1 = 0, rs2 = 0;

        if (std::holds_alternative<RType>(decoded))
        {
            auto r_type = std::get<RType>(decoded);
            rs1 = r_type.rs1;
            rs2 = r_type.rs2;
        }
        else if (std::holds_alternative<IType>(decoded))
        {
            auto i_type = std::get<IType>(decoded);
            rs1 = i_type.rs1;
        }

        if (pipeline.execute.valid)
        {
            auto &exec_instr = pipeline.execute.instruction;
            if (std::holds_alternative<RType>(exec_instr))
            {
                auto exec_r_type = std::get<RType>(exec_instr);
                if (exec_r_type.rd == rs1 || exec_r_type.rd == rs2)
                {
                    return true; // Hazard detected
                }
            }
        }
    }

    return false; // No hazard detected
}

/**
 * @brief Print the CPU registers.
 */
void CPU::print_registers() const
{
    std::cout << "PC: 0x" << Memory::to_hex_string(pc) << std::endl;
    for (size_t i = 0; i < 32; ++i)
    {
        std::cout << std::left << std::setw(4) << registers[i].name << ": "
                  << std::right << std::setw(8) << std::setfill(' ') << Memory::to_hex_string(registers[i].value) << std::endl;
    }
    std::cout << std::endl;
}

/**
 * @brief Execute the write-back stage.
 *
 * @param pipeline The pipeline state.
 */
void CPU::write_back()
{
    std::unique_lock<std::mutex> lock(write_back_mutex);
    write_back_cv.wait(lock, [this]
                       { return pipeline.memory.valid; });

    if (pipeline.memory.valid)
    {
        auto &instr = pipeline.memory.instruction;
        pipeline.write_back.pc = pipeline.memory.pc;
        pipeline.write_back.valid = true;
        pipeline.memory.valid = false;

        if (std::holds_alternative<RType>(instr))
        {
            auto r_type = std::get<RType>(instr);
            registers[r_type.rd].value = pipeline.execute.alu_result;
            LOG_DEBUG("Write-back R-Type: x" + std::to_string(r_type.rd) + " = " + std::to_string(pipeline.execute.alu_result));
        }
        else if (std::holds_alternative<IType>(instr))
        {
            if (static_cast<Opcode>(pipeline.fetch.instruction & 0x7F) == Opcode::I_TYPE_ALU)
            {
                auto i_type = std::get<IType>(instr);
                registers[i_type.rd].value = pipeline.execute.alu_result;
                LOG_DEBUG("Write-back I-Type: x" + std::to_string(i_type.rd) + " = " + Memory::to_hex_string(pipeline.execute.alu_result));
            }
            else
            {
                auto i_type = std::get<IType>(instr);
                registers[i_type.rd].value = pipeline.memory.result;
                LOG_DEBUG("Write-back I-Type: x" + std::to_string(i_type.rd) + " = " + Memory::to_hex_string(pipeline.memory.result));
            }
        }
        else if (std::holds_alternative<UType>(instr))
        {
            auto u_type = std::get<UType>(instr);
            registers[u_type.rd].value = pipeline.execute.alu_result;
            LOG_DEBUG("Write-back U-Type: x" + std::to_string(u_type.rd) + " = " + std::to_string(pipeline.execute.alu_result));
        }
    }
}

void CPU::start_threads()
{
    fetch_thread = std::thread(&CPU::fetch, this);
    decode_thread = std::thread(&CPU::decode, this);
    execute_thread = std::thread(&CPU::execute, this);
    mem_thread = std::thread(&CPU::mem, this);
    write_back_thread = std::thread(&CPU::write_back, this);

    LOG_DEBUG("Started CPU threads");
}

void CPU::stop_threads()
{
    fetch_thread.join();
    decode_thread.join();
    execute_thread.join();
    mem_thread.join();
    write_back_thread.join();

    LOG_DEBUG("Stopped CPU threads");
}
