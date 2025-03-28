#ifndef CPU_H
#define CPU_H

#include <array>
#include <variant>

#include "memory.h"

/**
 * @brief Enum for R-Type funct3 values.
 *
 * Represents the funct3 field for R-Type instructions, which determines
 * the specific operation to be performed.
 */
enum class RTypeFunct3 : uint8_t {
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
 *
 * Represents the funct3 field for I-Type instructions, which determines
 * the specific operation to be performed.
 */
enum class ITypeFunct3 : uint8_t {
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
 *
 * Represents the funct3 field for S-Type instructions, which determines
 * the specific memory store operation.
 */
enum class STypeFunct3 : uint8_t { SB = 0x0, SH = 0x1, SW = 0x2 };

/**
 * @brief Enum for B-Type funct3 values.
 *
 * Represents the funct3 field for B-Type instructions, which determines
 * the specific branch condition.
 */
enum class BTypeFunct3 : uint8_t {
    BEQ = 0x0,
    BNE = 0x1,
    BLT = 0x4,
    BGE = 0x5,
    BLTU = 0x6,
    BGEU = 0x7
};

/**
 * @brief Enum for funct7 values.
 *
 * Represents the funct7 field for R-Type instructions, which determines
 * additional operation details.
 */
enum class Funct7 : uint8_t {
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
 *
 * Represents the opcode field, which determines the instruction type.
 */
enum class Opcode : uint8_t {
    R_TYPE = 0x33,
    I_TYPE_LOAD = 0x03,
    I_TYPE_ALU = 0x13,
    JALR = 0x67,
    S_TYPE = 0x23,
    B_TYPE = 0x63,
    J_TYPE = 0x6F,
    U_TYPE = 0x37
};

/**
 * @brief Enum for status flags.
 *
 * Represents the status flags used to indicate the CPU's state, such as
 * whether it is halted or encountered a division by zero.
 */
enum class StatusFlags : uint32_t {
    HALT = 1 << 0,
    DIV_ZERO = 1 << 1,
    // Add other status flags as needed
};

/**
 * @brief Struct for R-Type instructions.
 *
 * Represents the fields of an R-Type instruction, including funct3, funct7,
 * destination register (rd), and source registers (rs1, rs2).
 */
struct RType {
    RTypeFunct3 funct3;  ///< Function 3 field
    Funct7 funct7;       ///< Function 7 field
    uint8_t rd;          ///< Destination register
    uint8_t rs1;         ///< Source register 1
    uint8_t rs2;         ///< Source register 2
};

/**
 * @brief Struct for I-Type instructions.
 *
 * Represents the fields of an I-Type instruction, including funct3,
 * destination register (rd), source register (rs1), and immediate value.
 */
struct IType {
    ITypeFunct3 funct3;  ///< Function 3 field
    uint8_t rd;          ///< Destination register
    uint8_t rs1;         ///< Source register 1
    int32_t imm;         ///< Immediate value
};

/**
 * @brief Struct for J-Type instructions.
 *
 * Represents the fields of a J-Type instruction, including destination
 * register (rd) and immediate value.
 */
struct JType {
    uint8_t rd;   ///< Destination register
    int32_t imm;  ///< Immediate value
};

/**
 * @brief Struct for S-Type instructions.
 *
 * Represents the fields of an S-Type instruction, including funct3,
 * source registers (rs1, rs2), and immediate value.
 */
struct SType {
    STypeFunct3 funct3;  ///< Function 3 field
    uint8_t rs1;         ///< Source register 1
    uint8_t rs2;         ///< Source register 2
    int32_t imm;         ///< Immediate value
};

/**
 * @brief Struct for B-Type instructions.
 *
 * Represents the fields of a B-Type instruction, including funct3,
 * source registers (rs1, rs2), and immediate value.
 */
struct BType {
    BTypeFunct3 funct3;  ///< Function 3 field
    uint8_t rs1;         ///< Source register 1
    uint8_t rs2;         ///< Source register 2
    int32_t imm;         ///< Immediate value
};

/**
 * @brief Struct for U-Type instructions.
 *
 * Represents the fields of a U-Type instruction, including destination
 * register (rd) and immediate value.
 */
struct UType {
    uint8_t rd;   ///< Destination register
    int32_t imm;  ///< Immediate value
};

// Pipeline stages
struct FetchStage {
    uint32_t instruction;
    uint32_t pc;
    bool valid = false;
};

struct DecodeStage {
    std::variant<RType, IType, SType, BType, JType, UType> instruction;
    uint32_t pc;
    bool valid = false;
};

struct ExecuteStage {
    std::variant<RType, IType, SType, BType, JType, UType> instruction;
    uint32_t pc;
    uint32_t alu_result;
    bool valid = false;
};

struct MemoryStage {
    std::variant<RType, IType, SType, BType, JType, UType> instruction;
    uint32_t pc;
    uint32_t result;
    bool valid = false;
};

struct WriteBackStage {
    std::variant<RType, IType, SType, BType, JType, UType> instruction;
    uint32_t pc;
    uint32_t rd;
    uint32_t result;
    bool valid = false;
};

// Pipeline state
struct Pipeline {
    FetchStage fetch;
    DecodeStage decode;
    ExecuteStage execute;
    MemoryStage memory;
    WriteBackStage write_back;
    bool stall = false;  // Stall signal
};

/**
 * @brief Struct representing a register with its value and name.
 *
 * Each register has a name (e.g., "x0", "ra") and a 32-bit value.
 */
struct Register {
    const char *name;
    uint32_t value;
};

/**
 * @brief Class representing the CPU.
 *
 * The CPU class emulates a RISC-V processor, including its pipeline stages
 * (fetch, decode, execute, memory, write-back) and register file. It supports
 * various instruction types and handles hazards, forwarding, and status flags.
 */
class CPU {
   public:
    /**
     * @brief Construct a new CPU object.
     *
     * @param memory Reference to the memory object.
     */
    CPU(Memory &memory);

    /**
     * @brief Fetch the next instruction from memory.
     *
     * Fetches the instruction at the current program counter (PC) and updates
     * the pipeline fetch stage.
     */
    void fetch();

    /**
     * @brief Decode the fetched instruction.
     *
     * Decodes the instruction fetched in the pipeline fetch stage and
     * determines its type (e.g., R-Type, I-Type, etc.).
     */
    void decode();

    /**
     * @brief Execute the decoded instruction.
     *
     * Executes the instruction in the pipeline decode stage. Handles ALU
     * operations, branching, and other instruction-specific logic.
     */
    void execute();

    /**
     * @brief Execute the memory stage.
     *
     * Handles memory operations such as loading and storing data.
     */
    void mem();

    /**
     * @brief Execute the write-back stage.
     *
     * Writes the results of executed instructions back to the appropriate
     * registers.
     */
    void write_back();

    /**
     * @brief Execute an I-Type ALU instruction.
     *
     * Performs ALU operations for I-Type instructions such as ADDI, SLTI, etc.
     *
     * @param instr The decoded I-Type instruction.
     * @return uint32_t The result of the ALU operation.
     */
    uint32_t execute_i_type(const IType &instr);

    /**
     * @brief Execute a store instruction.
     *
     * Handles memory store operations for S-Type instructions.
     *
     * @param instr The decoded S-Type instruction.
     */
    void execute_s_type(const SType &instr);

    /**
     * @brief Execute an R-Type instruction.
     *
     * Performs ALU operations for R-Type instructions such as ADD, SUB, etc.
     *
     * @param instr The decoded R-Type instruction.
     * @return uint32_t The result of the ALU operation.
     */
    uint32_t execute_r_type(const RType &instr);

    /**
     * @brief Execute a B-Type instruction.
     *
     * Handles branching logic for B-Type instructions such as BEQ and BNE.
     *
     * @param instr The decoded B-Type instruction.
     */
    void execute_b_type(const BType &instr);

    /**
     * @brief Execute a J-Type instruction.
     *
     * Handles jump instructions such as JAL.
     *
     * @param instr The decoded J-Type instruction.
     */
    void execute_j_type(const JType &instr);

    /**
     * @brief Execute a U-Type instruction.
     *
     * Handles upper immediate instructions such as LUI.
     *
     * @param instr The decoded U-Type instruction.
     * @return uint32_t The result of the ALU operation.
     */
    uint32_t execute_u_type(const UType &instr);

    /**
     * @brief Set the program counter.
     *
     * Updates the program counter (PC) to the specified address.
     *
     * @param address The address to set the program counter to.
     */
    void set_pc(uint32_t address);

    /**
     * @brief Get the program counter.
     *
     * @return uint32_t The current value of the program counter (PC).
     */
    uint32_t get_pc() const;

    /**
     * @brief Get the text size.
     *
     * Retrieves the size of the text segment from memory.
     *
     * @return uint32_t The size of the text segment.
     */
    uint32_t get_text_size() const;

    /**
     * @brief Set the stack pointer.
     *
     * Updates the stack pointer (SP) register to the specified address.
     *
     * @param address The address to set the stack pointer to.
     */
    void set_sp(uint32_t address);

    /**
     * @brief Check for data hazards and set stall signals.
     *
     * @return true if a stall is needed, false otherwise.
     */
    bool detect_hazard();

    /**
     * @brief Print the CPU registers.
     *
     * Outputs the current values of all CPU registers and the program counter
     * (PC).
     */
    void print_registers() const;

    /**
     * @brief Check if the fetch stage can proceed.
     *
     * Determines if the fetch stage is ready to fetch the next instruction.
     *
     * @return true if the fetch stage can proceed, false otherwise.
     */
    bool can_fetch();

    /**
     * @brief Check if the decode stage can proceed.
     *
     * Determines if the decode stage is ready to decode the fetched instruction.
     *
     * @return true if the decode stage can proceed, false otherwise.
     */
    bool can_decode();

    /**
     * @brief Check if the execute stage can proceed.
     *
     * Determines if the execute stage is ready to execute the decoded instruction.
     *
     * @return true if the execute stage can proceed, false otherwise.
     */
    bool can_execute();

    /**
     * @brief Check if the memory stage can proceed.
     *
     * Determines if the memory stage is ready to perform memory operations.
     *
     * @return true if the memory stage can proceed, false otherwise.
     */
    bool can_mem();

    /**
     * @brief Check if the write-back stage can proceed.
     *
     * Determines if the write-back stage is ready to write results back to registers.
     *
     * @return true if the write-back stage can proceed, false otherwise.
     */
    bool can_write_back();

    /**
     * @brief Get the status register.
     *
     * @return uint32_t The status register.
     */
    uint32_t get_status() const;

    /**
     * @brief Set the status register.
     *
     * @param status The status to set.
     */
    void set_status(uint32_t status);

    /**
     * @brief Set a specific status flag.
     *
     * @param flag The status flag to set.
     */
    void set_status_flag(StatusFlags flag);

    /**
     * @brief Clear a specific status flag.
     *
     * @param flag The status flag to clear.
     */
    void clear_status_flag(StatusFlags flag);

    /**
     * @brief Check if the CPU is halted.
     *
     * Determines if the CPU has been halted based on the status flags.
     *
     * @return true if the CPU is halted, false otherwise.
     */
    bool is_halted() const;

    /**
     * @brief Forward a register value.
     *
     * Implements forwarding logic to resolve data hazards in the pipeline.
     *
     * @param rs The source register index.
     * @return uint32_t The forwarded value.
     */
    uint32_t forward_value(uint8_t rs);

   private:
    Memory &memory;                      ///< Reference to the memory object.
    Pipeline pipeline;                   ///< The pipeline state.
    uint32_t pc;                         ///< Program Counter.
    uint32_t status;                     ///< Status register.
    std::array<Register, 32> registers;  ///< Registers with names.
    static constexpr std::array<const char *, 32> registerNames = {
        "zero", "ra", "sp", "gp", "tp",  "t0",  "t1", "t2", "s0", "s1", "a0",
        "a1",   "a2", "a3", "a4", "a5",  "a6",  "a7", "s2", "s3", "s4", "s5",
        "s6",   "s7", "s8", "s9", "s10", "s11", "t3", "t4", "t5", "t6"};
};

#endif
