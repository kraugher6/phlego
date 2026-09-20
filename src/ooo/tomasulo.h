#ifndef TOMASULO_H
#define TOMASULO_H

#include <vector>
#include <string>
#include <cstdint>
#include <array>
#include "core/decoder.h"

namespace phlego {

// --- Reorder Buffer (ROB) ---
struct ROBEntry {
    bool valid = false;
    bool ready = false;
    uint32_t pc = 0;
    uint8_t dest_reg = 0;
    uint32_t value = 0;
    DecodedInstruction dec;
    
    void clear() { valid = false; ready = false; pc = 0; dest_reg = 0; value = 0; }
};

// --- Reservation Station (RS) ---
struct ReservationStation {
    std::string name;
    bool busy = false;
    uint8_t op = 0;
    
    // Operands: value or ROB index (if waiting)
    uint32_t vj = 0, vk = 0;
    int32_t qj = -1, qk = -1; // -1 if ready, else index in ROB
    
    uint32_t dest_rob_idx = 0;
    uint32_t pc = 0;
    uint32_t remaining_cycles = 0;

    void clear() { busy = false; op = 0; vj = 0; vk = 0; qj = -1; qk = -1; dest_rob_idx = 0; pc = 0; remaining_cycles = 0; }
};

class TomasuloUnit {
public:
    TomasuloUnit(uint32_t rob_size = 16);

    // 1. Issue: Try to allocate ROB entry and RS
    bool can_issue() const;
    void issue(const DecodedInstruction& dec, uint32_t pc, const std::array<uint32_t, 32>& rf_vals);

    // 2. Execute: Instructions whose operands are ready start/continue execution
    void execute_tick();

    // 3. Write Result: Broadcast on CDB
    void write_result();

    // 4. Commit: Head of ROB writes to Register File in order
    struct CommitInfo {
        bool valid = false;
        uint8_t rd = 0;
        uint32_t value = 0;
        uint32_t pc = 0;
    };
    CommitInfo commit();

    void dump_state() const;

private:
    // Functional Units (mapped to RS)
    std::vector<ReservationStation> rs_list;
    
    // ROB (Circular Buffer)
    std::vector<ROBEntry> rob;
    uint32_t rob_head = 0;
    uint32_t rob_tail = 0;
    uint32_t rob_count = 0;

    // RAT (Register Alias Table): register -> last ROB index producing it (-1 if in RF)
    int32_t rat[32];

    uint32_t get_latency(const DecodedInstruction& dec);
    ReservationStation* find_free_rs(const DecodedInstruction& dec);
};

} // namespace phlego

#endif
