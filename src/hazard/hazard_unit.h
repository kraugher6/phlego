#ifndef HAZARD_UNIT_H
#define HAZARD_UNIT_H

#include <cstdint>
#include "core/pipeline_regs.h"

namespace phlego {

struct HazardSignals {
    bool stall_if = false;
    bool stall_id = false;
    bool flush_id = false;
    bool flush_ex = false;
};

class HazardUnit {
public:
    static HazardSignals detect(
        const DecodedInstruction& id_dec,
        const ID_EX& id_ex,
        const EX_MEM& ex_mem,
        const MEM_WB& mem_wb,
        bool data_forwarding = true
    );
};

} // namespace phlego

#endif
