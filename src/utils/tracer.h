#ifndef TRACER_H
#define HAZARD_UNIT_H

#include <vector>
#include <string>
#include <cstdint>
#include <fstream>
#include "core/pipeline_regs.h"

namespace phlego {

struct CycleState {
    uint64_t cycle;
    uint32_t pc_if, pc_id, pc_ex, pc_mem, pc_wb;
    bool valid_if, valid_id, valid_ex, valid_mem, valid_wb;
    std::vector<uint32_t> registers;
    std::string disassembly_id; // Just for visualization
    bool stall, flush, fwd;
};

class Tracer {
public:
    Tracer(const std::string& filename);
    ~Tracer();

    void add_cycle(const CycleState& state);
    void save();

private:
    std::string filename;
    std::vector<CycleState> trace;
};

} // namespace phlego

#endif
