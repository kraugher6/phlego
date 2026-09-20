#include "hazard/forwarding_unit.h"

namespace phlego {

ForwardSignals ForwardingUnit::detect(
    const ID_EX_Entry& entry,
    const EX_MEM& ex_mem,
    const MEM_WB& mem_wb
) {
    ForwardSignals signals;

    if (!entry.valid) return signals;

    uint8_t rs1 = entry.dec.rs1;
    uint8_t rs2 = entry.dec.rs2;

    auto writes_rd = [](const DecodedInstruction& dec) {
        return (dec.type == InstructionType::R_TYPE || 
                dec.type == InstructionType::I_TYPE || 
                dec.type == InstructionType::U_TYPE || 
                dec.type == InstructionType::J_TYPE);
    };

    // --- Forwarding for rs1 ---
    // Priority 1: EX_MEM (most recent)
    // In superscalar, if multiple slots write to the same register, 
    // the instruction in the HIGHER slot index is more recent.
    for (int i = MAX_WIDTH - 1; i >= 0; --i) {
        if (ex_mem.slots[i].valid && writes_rd(ex_mem.slots[i].dec) && ex_mem.slots[i].rd != 0 && ex_mem.slots[i].rd == rs1) {
            signals.src_a = ForwardSource::EX_MEM_STAGE;
            signals.slot_a = i;
            break;
        }
    }
    // Priority 2: MEM_WB
    if (signals.src_a == ForwardSource::REGISTER_FILE) {
        for (int i = MAX_WIDTH - 1; i >= 0; --i) {
            if (mem_wb.slots[i].valid && writes_rd(mem_wb.slots[i].dec) && mem_wb.slots[i].rd != 0 && mem_wb.slots[i].rd == rs1) {
                signals.src_a = ForwardSource::MEM_WB_STAGE;
                signals.slot_a = i;
                break;
            }
        }
    }

    // --- Forwarding for rs2 ---
    for (int i = MAX_WIDTH - 1; i >= 0; --i) {
        if (ex_mem.slots[i].valid && writes_rd(ex_mem.slots[i].dec) && ex_mem.slots[i].rd != 0 && ex_mem.slots[i].rd == rs2) {
            signals.src_b = ForwardSource::EX_MEM_STAGE;
            signals.slot_b = i;
            break;
        }
    }
    if (signals.src_b == ForwardSource::REGISTER_FILE) {
        for (int i = MAX_WIDTH - 1; i >= 0; --i) {
            if (mem_wb.slots[i].valid && writes_rd(mem_wb.slots[i].dec) && mem_wb.slots[i].rd != 0 && mem_wb.slots[i].rd == rs2) {
                signals.src_b = ForwardSource::MEM_WB_STAGE;
                signals.slot_b = i;
                break;
            }
        }
    }

    return signals;
}

} // namespace phlego
