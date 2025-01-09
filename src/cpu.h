#ifndef CPU_H
#define CPU_H

#include "memory.h"
#include <variant>
#include <array>
#include <iostream>

/**
 * @brief Enum for R-Type funct3 values.
 */
enum class RTypeFunct3 : uint8_t
{
    ADD = 0x0,
    SUB = 0x0,
    SLL = 0x1,
    SLT = 0x2,
    SLTU = 0x3,
    XOR = 0x4,
    SRL = 0x5,
    SRA = 0x5,
    OR = 0x6,
    AND = 0x7,
    MUL = 0x0,
    MULH = 0x1,
    MULHSU = 0x2,
    MULHU = 0x3,
    DIV = 0x4,
    DIVU = 0x5,
    REM = 0x6,
    REMU = 0x7
};

/**
 * @brief Enum for I-Type funct3 values.
 */
enum class ITypeFunct3 : uint8_t
{
    ADDI = 0x0,
    SLTI = 0x2,
    SLTIU = 0x3,
    XORI = 0x4,
    ORI = 0x6,
    ANDI = 0x7,
    SLLI = 0x1,
    SRLI = 0x5,
    SRAI = 0x5,
    LB = 0x0,
    LH = 0x1,
    LW = 0x2,
    LBU = 0x4,
    LHU = 0x5
};

/**
 * @brief Enum for S-Type funct3 values.
 */
enum class STypeFunct3 : uint8_t
{
    SB = 0x0,
    SH = 0x1,
    SW = 0x2
};

/**
 * @brief Enum for B-Type funct3 values.
 */
enum class BTypeFunct3 : uint8_t
{
    BEQ = 0x0,
    BNE = 0x1,
    BLT = 0x4,
    BGE = 0x5,
    BLTU = 0x6,
    BGEU = 0x7
};

/**
 * @brief Enum for funct7 values.
 */
enum class Funct7 : uint8_t
{
    ADD = 0x00,
    SUB = 0x20,
    SLL = 0x00,
    SLT = 0x00,
    SLTU = 0x00,
    XOR = 0x00,
    SRL = 0x00,
    SRA = 0x20,
    OR = 0x00,
    AND = 0x00,
    MUL = 0x01,
    MULH = 0x01,
    MULHSU = 0x01,
    MULHU = 0x01,
    DIV = 0x01,
    DIVU = 0x01,
    REM = 0x01,
    REMU = 0x01
};

/**
 * @brief Enum for opcodes.
 */
enum class Opcode : uint8_t
{
    R_TYPE = 0x33,
    I_TYPE_LOAD = 0x03,
    I_TYPE_ALU = 0x13,
    JALR = 0x67,
    S_TYPE = 0x23,
    B_TYPE = 0x63,
    J_TYPE = 0x6F
};

/**
 * @brief Enum for register names.
 */
enum class RegisterName : uint8_t
{
    ZERO, RA, SP, GP, TP, T0, T1, T2,
    S0, S1, A0, A1, A2, A3, A4, A5,
    A6, A7, S2, S3, S4, S5, S6, S7,
    S8, S9, S10, S11, T3, T4, T5, T6
};

/**
 * @brief Struct for R-Type instructions.
 */
struct RType
{
    RTypeFunct3 funct3; ///< Function 3 field
    Funct7 funct7;      ///< Function 7 field
    uint8_t rd;         ///< Destination register
    uint8_t rs1;        ///< Source register 1
    uint8_t rs2;        ///< Source register 2
};

/**
 * @brief Struct for I-Type instructions.
 */
struct IType
{
    ITypeFunct3 funct3; ///< Function 3 field
    uint8_t rd;         ///< Destination register
    uint8_t rs1;        ///< Source register 1
    int32_t imm;        ///< Immediate value
};

/**
 * @brief Struct for J-Type instructions.
 */
struct JType
{
    uint8_t rd;  ///< Destination register
    int32_t imm; ///< Immediate value
};

/**
 * @brief Struct for S-Type instructions.
 */
struct SType
{
    STypeFunct3 funct3; ///< Function 3 field
    uint8_t rs1;        ///< Source register 1
    uint8_t rs2;        ///< Source register 2
    int32_t imm;        ///< Immediate value
};

/**
 * @brief Struct for B-Type instructions.
 */
struct BType
{
    BTypeFunct3 funct3; ///< Function 3 field
    uint8_t rs1;        ///< Source register 1
    uint8_t rs2;        ///< Source register 2
    int32_t imm;        ///< Immediate value
};

// Pipeline stages
struct FetchStage
{
    uint32_t instruction;
    uint32_t pc;
    bool valid = false;
};

struct DecodeStage
{
    std::variant<RType, IType, SType, BType, JType> decoded_instruction;
    uint32_t pc;
    bool valid = false;
};

struct ExecuteStage
{
    std::variant<RType, IType, SType, BType, JType> instruction;
    uint32_t pc;
    uint32_t alu_result;
    bool valid = false;
};

struct MemoryStage
{
    std::variant<RType, IType, SType, BType, JType> instruction;
    uint32_t pc;
    uint32_t result;
    bool valid = false;
};

struct WriteBackStage
{
    uint32_t pc;
    uint32_t rd;
    uint32_t result;
    bool valid = false;
};

// Pipeline state
struct Pipeline
{
    FetchStage fetch;
    DecodeStage decode;
    ExecuteStage execute;
    MemoryStage memory;
    WriteBackStage write_back;
    bool stall = false; // Stall signal
};

/**
 * @brief Struct representing a register with its value and name.
 */
struct Register
{
    uint32_t value;
    RegisterName name;
};

/**
 * @brief Class representing the CPU.
 */
class CPU
{
public:
    /**
     * @brief Construct a new CPU object.
     *
     * @param memory Reference to the memory object.
     */
    CPU(Memory &memory);

    /**
     * @brief Execute the fetch-decode-run cycle.
     */
    void run();

    /**
     * @brief Fetch the next instruction from memory.
     *
     * @param pipeline The pipeline state.
     */
    void fetch(Pipeline &pipeline);

    /**
     * @brief Decode the fetched instruction.
     *
     * @param pipeline The pipeline state.
     */
    void decode(Pipeline &pipeline);

    /**
     * @brief Execute the decoded instruction.
     *
     * @param pipeline The pipeline state.
     */
    void execute(Pipeline &pipeline);

    /**
     * @brief Execute the memory stage.
     *
     * @param pipeline The pipeline state.
     */
    void mem(Pipeline &pipeline);

    /**
     * @brief Execute the write-back stage.
     *
     * @param pipeline The pipeline state.
     */
    void write_back(Pipeline &pipeline);

    /**
     * @brief Execute an I-Type ALU instruction.
     *
     * @param pipeline The pipeline state.
     * @param instr The decoded I-Type instruction.
     */
    uint32_t execute_i_type(const IType &instr);

    /**
     * @brief Execute a store instruction.
     *
     * @param instr The decoded S-Type instruction.
     */
    void execute_s_type(const SType &instr);

    /**
     * @brief Execute an R-Type instruction.
     *
     * @param instr The decoded R-Type instruction.
     * @return uint32_t The result of the ALU operation.
     */
    uint32_t execute_r_type(const RType &instr);

    /**
     * @brief Execute a B-Type instruction.
     *
     * @param instr The decoded B-Type instruction.
     */
    void execute_b_type(const BType &instr);

    /**
     * @brief Execute a J-Type instruction.
     *
     * @param instr The decoded J-Type instruction.
     */
    void execute_j_type(const JType &instr);

    /**
     * @brief Set the program counter.
     *
     * @param address The address to set the program counter to.
     */
    void set_pc(uint32_t address);

    /**
     * @brief Set the stack pointer.
     *
     * @param address The address to set the stack pointer to.
     */
    void set_sp(uint32_t address);

    /**
     * @brief Check for data hazards and set stall signals.
     *
     * @param pipeline The pipeline state.
     * @return true if a stall is needed, false otherwise.
     */
    bool detect_hazard(const Pipeline &pipeline);

    /**
     * @brief Print the CPU registers.
     */
    void print_registers() const;

private:
    Memory &memory;                     ///< Reference to the memory object.
    uint32_t pc;                        ///< Program Counter.
    std::array<Register, 32> registers; ///< Registers with names.
};

#endif
