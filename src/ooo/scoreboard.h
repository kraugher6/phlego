#ifndef SCOREBOARD_H
#define SCOREBOARD_H

#include <vector>
#include <string>
#include <cstdint>
#include "core/decoder.h"

namespace phlego {

enum class FUType { INTEGER, MULTIPLIER, DIVIDER, MEMORY };

struct FunctionalUnit {
    std::string name;
    FUType type;
    bool busy = false;
    uint8_t op = 0;
    int8_t fi = -1, fj = -1, fk = -1; // Registers: Dest, Src1, Src2
    std::string qj = "", qk = "";     // FUs producing source registers
    bool rj = true, rk = true;        // Source registers ready
    uint32_t remaining_cycles = 0;
    uint32_t pc = 0;                  // PC of instruction currently in FU
    
    void clear() {
        busy = false; op = 0; fi = -1; fj = -1; fk = -1;
        qj = ""; qk = ""; rj = true; rk = true;
        remaining_cycles = 0; pc = 0;
    }
};

class Scoreboard {
public:
    Scoreboard();

    // Check if an instruction can be issued (WAW check + Structural check)
    bool can_issue(const DecodedInstruction& dec);
    void issue(const DecodedInstruction& dec, uint32_t pc);

    // Read operands (RAW check)
    std::vector<std::string> get_ready_fus();
    void read_operands(const std::string& fu_name);

    // Execute (Decrement cycle counts)
    void tick();
    std::vector<std::string> get_finished_fus();

    // Write result (WAR check)
    bool can_write_result(const std::string& fu_name);
    void write_result(const std::string& fu_name);

    void dump_state() const;

private:
    std::vector<FunctionalUnit> fus;
    std::string register_result[32]; // Which FU will write to each register

    FunctionalUnit* get_fu(const std::string& name);
    FUType get_required_fu_type(const DecodedInstruction& dec);
};

} // namespace phlego

#endif
