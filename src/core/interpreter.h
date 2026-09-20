#ifndef INTERPRETER_H
#define INTERPRETER_H

#include "core/memory.h"
#include "core/register_file.h"
#include "core/decoder.h"

namespace phlego {

class Interpreter {
public:
    Interpreter(Memory& mem);

    void set_pc(uint32_t pc_val) { pc = pc_val; }
    uint32_t get_pc() const { return pc; }
    void set_register(uint8_t reg_idx, uint32_t val) { rf.write(reg_idx, val); }
    uint32_t get_register(uint8_t reg) const { return rf.read(reg); }

    bool step();
    void run();

    void dump_state() const;

private:
    Memory& memory;
    RegisterFile rf;
    uint32_t pc;
    uint64_t inst_count;
    bool halted;

    void execute(const DecodedInstruction& instr);
};

} // namespace phlego

#endif
