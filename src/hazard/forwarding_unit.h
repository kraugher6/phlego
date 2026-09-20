#ifndef FORWARDING_UNIT_H
#define FORWARDING_UNIT_H

#include <cstdint>
#include "core/pipeline_regs.h"

namespace phlego {

enum class ForwardSource {
    REGISTER_FILE,
    EX_MEM_STAGE,
    MEM_WB_STAGE
};

struct ForwardSignals {
    ForwardSource src_a = ForwardSource::REGISTER_FILE;
    ForwardSource src_b = ForwardSource::REGISTER_FILE;
    int slot_a = -1; // Which slot in the stage to forward from
    int slot_b = -1;
};

class ForwardingUnit {
public:
    static ForwardSignals detect(
        const ID_EX_Entry& entry,
        const EX_MEM& ex_mem,
        const MEM_WB& mem_wb
    );
};

} // namespace phlego

#endif
