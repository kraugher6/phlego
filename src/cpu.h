#ifndef CPU_H
#define CPU_H

#include "memory.h"
#include <variant>

/**
 * @brief Struct for R-Type instructions.
 */
struct RType {
    uint8_t funct3; ///< Function 3 field
    uint8_t funct7; ///< Function 7 field
    uint8_t rd;     ///< Destination register
    uint8_t rs1;    ///< Source register 1
    uint8_t rs2;    ///< Source register 2
};

/**
 * @brief Struct for I-Type instructions.
 */
struct IType {
    uint8_t funct3; ///< Function 3 field
    uint8_t rd;     ///< Destination register
    uint8_t rs1;    ///< Source register 1
    int32_t imm;    ///< Immediate value
};

/**
 * @brief Struct for J-Type instructions.
 */
struct JType {
    uint8_t rd;     ///< Destination register
    int32_t imm;    ///< Immediate value
};

/**
 * @brief Struct for S-Type instructions.
 */
struct SType {
    int32_t imm;    ///< Immediate value
    uint8_t rs1;    ///< Source register 1
    uint8_t rs2;    ///< Source register 2
    uint8_t funct3; ///< Function 3 field
};

/**
 * @brief Struct for B-Type instructions.
 */
struct BType {
    int32_t imm;    ///< Immediate value
    uint8_t rs1;    ///< Source register 1
    uint8_t rs2;    ///< Source register 2
    uint8_t funct3; ///< Function 3 field
};

/**
 * @brief Enum for opcodes.
 */
enum class Opcode : uint8_t {
    R_TYPE = 0x33,
    I_TYPE_LOAD = 0x03,
    I_TYPE_ALU = 0x13,
    JALR = 0x67,
    S_TYPE = 0x23,
    B_TYPE = 0x63,
    J_TYPE = 0x6F
};

/**
 * @brief Enum for funct3 values.
 */
enum class Funct3 : uint8_t {
    // R-Type and I-Type ALU
    ADD_SUB = 0x0,
    SLL = 0x1,
    SLT = 0x2,
    SLTU = 0x3,
    XOR = 0x4,
    SRL_SRA = 0x5,
    OR = 0x6,
    AND = 0x7,

    // I-Type Load
    LB = 0x0,
    LH = 0x1,
    LW = 0x2,
    LBU = 0x4,
    LHU = 0x5,

    // S-Type
    SB = 0x0,
    SH = 0x1,
    SW = 0x2,

    // B-Type
    BEQ = 0x0,
    BNE = 0x1,
    BLT = 0x4,
    BGE = 0x5,
    BLTU = 0x6,
    BGEU = 0x7
};

/**
 * @brief Class representing the CPU.
 */
class CPU {
public:
    /**
     * @brief Construct a new CPU object.
     *
     * @param memory Reference to the memory object.
     */
    CPU(Memory& memory);

    /**
     * @brief Execute the fetch-decode-run cycle.
     */
    void run();

    /**
     * @brief Decode an instruction.
     *
     * @param instruction The instruction to decode.
     * @return std::variant<RType, IType, SType, BType, JType> The decoded instruction.
     */
    std::variant<RType, IType, SType, BType, JType> decode(uint32_t instruction);

    /**
     * @brief Execute a load instruction.
     *
     * @param instr The decoded I-Type instruction.
     */
    void execute_load(const IType& instr);

    /**
     * @brief Execute an ALU instruction.
     *
     * @param instr The decoded I-Type instruction.
     */
    void execute_alu(const IType& instr);

    /**
     * @brief Execute a store instruction.
     *
     * @param instr The decoded S-Type instruction.
     */
    void execute_store(const SType& instr);

    /**
     * @brief Execute an R-Type instruction.
     *
     * @param instr The decoded R-Type instruction.
     */
    void execute_r_type(const RType& instr);

    /**
     * @brief Execute a B-Type instruction.
     *
     * @param instr The decoded B-Type instruction.
     */
    void execute_b_type(const BType& instr);

    /**
     * @brief Execute a J-Type instruction.
     *
     * @param instr The decoded J-Type instruction.
     */
    void execute_j_type(const JType& instr);

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
     * @brief Print the CPU registers.
     */
    void print_registers() const;

private:
    Memory& memory; ///< Reference to the memory object.
    uint32_t pc; ///< Program Counter.
    uint32_t registers[32]; ///< Registers.
};

#endif
