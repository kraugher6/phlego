#include "hazard/hazard_unit.h"

namespace phlego {

bool writes_rd(const DecodedInstruction& dec) {
    if (dec.type == InstructionType::S_TYPE || dec.type == InstructionType::B_TYPE) return false;
    return dec.rd != 0;
}

HazardSignals HazardUnit::detect(
    const DecodedInstruction& id_dec,
    const ID_EX& id_ex,
    const EX_MEM& ex_mem,
    const MEM_WB& mem_wb,
    bool data_forwarding
) {
    HazardSignals signals;

    bool id_uses_rs1 = (id_dec.type == InstructionType::R_TYPE || 
                        id_dec.type == InstructionType::I_TYPE || 
                        id_dec.type == InstructionType::S_TYPE || 
                        id_dec.type == InstructionType::B_TYPE);
    
    bool id_uses_rs2 = (id_dec.type == InstructionType::R_TYPE || 
                        id_dec.type == InstructionType::S_TYPE || 
                        id_dec.type == InstructionType::B_TYPE);

    uint8_t rs1 = id_dec.rs1;
    uint8_t rs2 = id_dec.rs2;

    // Load-Use Hazard in Superscalar:
    // Check all instructions currently in EX stage (id_ex slots)
    for (int i = 0; i < MAX_WIDTH; ++i) {
        if (id_ex.slots[i].valid && id_ex.slots[i].dec.opcode == 0x03) { // LOAD
            uint8_t rd_load = id_ex.slots[i].dec.rd;
            if (rd_load != 0 && ((id_uses_rs1 && rs1 == rd_load) || (id_uses_rs2 && rs2 == rd_load))) {
                signals.stall_if = true;
                signals.stall_id = true;
                signals.flush_ex = true;
                return signals; // One hazard is enough to stall the cycle
            }
        }
    }

    // If data forwarding is disabled, check ALL RAW hazards with EX stage (id_ex) and MEM stage (ex_mem)
    if (!data_forwarding) {
        // Check EX stage instructions
        for (int i = 0; i < MAX_WIDTH; ++i) {
            if (id_ex.slots[i].valid && writes_rd(id_ex.slots[i].dec)) {
                uint8_t rd_ex = id_ex.slots[i].dec.rd;
                if ((id_uses_rs1 && rs1 != 0 && rs1 == rd_ex) || 
                    (id_uses_rs2 && rs2 != 0 && rs2 == rd_ex)) {
                    signals.stall_if = true;
                    signals.stall_id = true;
                    signals.flush_ex = true;
                    return signals;
                }
            }
        }
        // Check MEM stage instructions
        for (int i = 0; i < MAX_WIDTH; ++i) {
            if (ex_mem.slots[i].valid && writes_rd(ex_mem.slots[i].dec)) {
                uint8_t rd_mem = ex_mem.slots[i].dec.rd;
                if ((id_uses_rs1 && rs1 != 0 && rs1 == rd_mem) || 
                    (id_uses_rs2 && rs2 != 0 && rs2 == rd_mem)) {
                    signals.stall_if = true;
                    signals.stall_id = true;
                    signals.flush_ex = true;
                    return signals;
                }
            }
        }
    }

    return signals;
}

} // namespace phlego
