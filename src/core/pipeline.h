#ifndef PIPELINE_H
#define PIPELINE_H

#include "core/memory.h"
#include "core/register_file.h"
#include "core/decoder.h"
#include "core/pipeline_regs.h"
#include "hazard/hazard_unit.h"
#include "hazard/forwarding_unit.h"
#include "prediction/branch_predictor.h"
#include "cache/cache.h"
#include "config/sim_config.h"
#include "utils/tracer.h"
#include "ooo/scoreboard.h"
#include "ooo/tomasulo.h"
#include <memory>

namespace phlego {

struct ExecutionMetrics {
    std::string config_name;
    uint64_t cycles = 0;
    uint64_t instructions = 0;
    uint64_t stalls_hazard = 0;
    uint64_t stalls_cache = 0;
    uint64_t flushes = 0;
    uint64_t forwarding_events = 0;
    double ipc = 0;
    double icache_hit_rate = 0;
    double dcache_hit_rate = 0;
};

class Pipeline {
public:
    Pipeline(Memory& mem, const SimConfig& cfg = SimConfig());

    void set_pc(uint32_t pc_val) { pc = pc_val; }
    uint32_t get_pc() const { return pc; }
    void set_register(uint8_t reg_idx, uint32_t val) { rf.write(reg_idx, val); }
    uint32_t get_register(uint8_t reg) const { return rf.read(reg); }

    void enable_tracing(const std::string& filename);

    bool tick(); 
    void dump_state() const;
    void print_pipeline_diagram() const;
    
    ExecutionMetrics get_metrics() const;

private:
    Memory& memory;
    SimConfig config;
    RegisterFile rf;
    std::unique_ptr<Tracer> tracer;
    Scoreboard scoreboard;
    TomasuloUnit tomasulo;
    
    uint32_t pc;
    uint64_t cycle_count;
    uint64_t inst_count;
    uint64_t stall_count;
    uint64_t forward_count;
    uint64_t flush_count;
    uint64_t cache_stall_count;
    bool halted;

    HazardSignals hazard_signals;
    ForwardSignals forward_signals;
    BranchPredictor predictor;
    
    Cache icache;
    Cache dcache;

    // Latches
    IF_ID if_id;
    ID_EX id_ex;
    EX_MEM ex_mem;
    MEM_WB mem_wb;

    IF_ID next_if_id;
    ID_EX next_id_ex;
    EX_MEM next_ex_mem;
    MEM_WB next_mem_wb;

    bool branch_mispredicted = false;
    uint32_t correct_target_pc = 0;
    uint32_t remaining_cache_stall = 0;

    void stage_fetch();
    void stage_decode();
    void stage_execute();
    void stage_memory();
    void stage_writeback();

    void tick_scoreboard();
    void tick_tomasulo();

    void record_trace();
    uint32_t compute_alu(const DecodedInstruction& dec, uint32_t op1, uint32_t op2);
};

} // namespace phlego

#endif
