#include "core/pipeline.h"
#include "logger.h"
#include <iostream>
#include <iomanip>

namespace phlego {

Pipeline::Pipeline(Memory& mem, const SimConfig& cfg) 
    : memory(mem), config(cfg), pc(0), cycle_count(0), inst_count(0), stall_count(0), forward_count(0), flush_count(0), cache_stall_count(0), halted(false),
      icache({"I-Cache", config.icache.size_bytes, config.icache.associativity, config.icache.line_size_bytes, config.icache.miss_penalty}),
      dcache({"D-Cache", config.dcache.size_bytes, config.dcache.associativity, config.dcache.line_size_bytes, config.dcache.miss_penalty})
{}

void Pipeline::enable_tracing(const std::string& filename) { tracer = std::make_unique<Tracer>(filename); }

void Pipeline::record_trace() {
    if (!tracer) return;
    CycleState state; state.cycle = cycle_count;
    state.pc_if = if_id.slots[0].pc; state.pc_id = if_id.slots[0].pc;
    state.pc_ex = id_ex.slots[0].pc; state.pc_mem = ex_mem.slots[0].pc; state.pc_wb = mem_wb.slots[0].pc;
    state.valid_if = if_id.slots[0].valid; state.valid_id = if_id.slots[0].valid;
    state.valid_ex = id_ex.slots[0].valid; state.valid_mem = ex_mem.slots[0].valid; state.valid_wb = mem_wb.slots[0].valid;
    state.stall = (remaining_cache_stall > 0); state.flush = branch_mispredicted; state.fwd = false;
    for (int i = 0; i < 32; ++i) state.registers.push_back(rf.read(i));
    tracer->add_cycle(state);
}

bool Pipeline::tick() {
    if (config.pipeline_type == PipelineType::TOMASULO) {
        tick_tomasulo();
        return !halted || if_id.count > 0;
    }
    if (config.pipeline_type == PipelineType::SCOREBOARD) {
        tick_scoreboard();
        return !halted || if_id.count > 0;
    }

    if (halted && if_id.count == 0 && id_ex.count == 0 && ex_mem.count == 0 && mem_wb.count == 0) return false;
    if (cycle_count >= config.max_cycles) return false;

    cycle_count++;
    if (remaining_cache_stall > 0) {
        remaining_cache_stall--; cache_stall_count++;
        print_pipeline_diagram(); record_trace();
        return true;
    }

    hazard_signals = {};
    next_if_id = if_id; next_id_ex.clear(); next_ex_mem.clear(); next_mem_wb.clear();

    stage_writeback();
    stage_memory();
    stage_execute();

    if (branch_mispredicted) {
        flush_count++; pc = correct_target_pc;
        next_if_id.clear(); next_id_ex.clear();
        branch_mispredicted = false;
    } else {
        stage_decode();
        stage_fetch();
    }

    if_id = next_if_id; id_ex = next_id_ex; ex_mem = next_ex_mem; mem_wb = next_mem_wb;
    print_pipeline_diagram(); record_trace();
    return true;
}

void Pipeline::tick_scoreboard() {
    cycle_count++;
    auto finished = scoreboard.get_finished_fus();
    for (const auto& fu_name : finished) {
        if (scoreboard.can_write_result(fu_name)) {
            scoreboard.write_result(fu_name);
            inst_count++;
        }
    }
    scoreboard.tick();
    auto ready = scoreboard.get_ready_fus();
    for (const auto& fu_name : ready) scoreboard.read_operands(fu_name);
    if (if_id.count == 0 && !halted) { stage_fetch(); if_id = next_if_id; }
    if (if_id.count > 0) {
        DecodedInstruction dec = Decoder::decode(if_id.slots[0].instr);
        if (scoreboard.can_issue(dec)) {
            scoreboard.issue(dec, if_id.slots[0].pc);
            if_id.clear();
        } else stall_count++;
    }
    if (cycle_count % 10 == 0) scoreboard.dump_state();
}

void Pipeline::tick_tomasulo() {
    cycle_count++;
    
    // 1. Commit
    TomasuloUnit::CommitInfo c = tomasulo.commit();
    if (c.valid) {
        if (c.rd != 0) rf.write(c.rd, c.value);
        inst_count++;
    }

    // 2. Write Result (CDB Broadcast)
    tomasulo.write_result();

    // 3. Execute
    tomasulo.execute_tick();

    // 4. Issue
    if (if_id.count == 0 && !halted) {
        stage_fetch();
        if_id = next_if_id;
    }

    if (if_id.count > 0) {
        DecodedInstruction dec = Decoder::decode(if_id.slots[0].instr);
        if (tomasulo.can_issue()) {
            tomasulo.issue(dec, if_id.slots[0].pc, rf.get_all_registers());
            if_id.clear();
        } else {
            stall_count++;
        }
    }

    if (cycle_count % 10 == 0) tomasulo.dump_state();
}

void Pipeline::stage_fetch() {
    if (halted) return;
    uint32_t width = config.issue_width;
    if (if_id.count >= width) return;
    next_if_id.clear();
    for (uint32_t i = 0; i < width; ++i) {
        if (config.icache.enabled) {
            uint32_t p = icache.access(pc, cycle_count);
            if (p > 0) { remaining_cache_stall = p; return; }
        }
        uint32_t instr = memory.read_word(pc);
        if (instr == 0 || instr == 0x00000073) { halted = true; break; }
        Prediction pred = {false, 0};
        if (config.branch_predictor != PredictorType::NONE) pred = predictor.predict(pc);
        next_if_id.slots[i] = {true, pc, instr, pred.taken, pred.target_pc};
        next_if_id.count++;
        if (pred.taken) { pc = pred.target_pc; break; } else pc += 4;
    }
}

void Pipeline::stage_decode() {
    if (if_id.count == 0) return;
    next_id_ex.clear();
    for (uint32_t i = 0; i < if_id.count; ++i) {
        const auto& entry = if_id.slots[i];
        if (!entry.valid) continue;
        DecodedInstruction dec = Decoder::decode(entry.instr);
        if (config.hazard_detection) {
            HazardSignals signals = HazardUnit::detect(dec, id_ex, ex_mem, mem_wb, config.data_forwarding);
            if (signals.stall_id) { hazard_signals = signals; stall_count++; break; }
            if (i > 0) {
                const auto& prev = next_id_ex.slots[i-1].dec;
                if ((dec.rs1 != 0 && dec.rs1 == prev.rd) || 
                    (dec.rs2 != 0 && dec.rs2 == prev.rd) ||
                    (dec.rd != 0 && dec.rd == prev.rd) ||
                    (dec.rd != 0 && (dec.rd == prev.rs1 || dec.rd == prev.rs2))) break;
            }
        }
        next_id_ex.slots[i] = {true, entry.pc, dec, rf.read(dec.rs1), rf.read(dec.rs2), entry.pred_taken, entry.pred_target};
        next_id_ex.count++;
    }
    uint32_t issued = next_id_ex.count;
    if (issued > 0) {
        IF_ID new_if_id; uint32_t remaining = 0;
        for (uint32_t j = issued; j < if_id.count; ++j) new_if_id.slots[remaining++] = if_id.slots[j];
        new_if_id.count = remaining; next_if_id = new_if_id;
    }
}

void Pipeline::stage_execute() {
    if (id_ex.count == 0) return;
    for (uint32_t i = 0; i < id_ex.count; ++i) {
        const auto& entry = id_ex.slots[i];
        if (!entry.valid) continue;
        uint32_t op1 = entry.rs1_val, op2_val = entry.rs2_val;
        if (config.data_forwarding) {
            ForwardSignals fwd = ForwardingUnit::detect(entry, ex_mem, mem_wb);
            if (fwd.src_a == ForwardSource::EX_MEM_STAGE) op1 = ex_mem.slots[fwd.slot_a].alu_result;
            else if (fwd.src_a == ForwardSource::MEM_WB_STAGE) op1 = mem_wb.slots[fwd.slot_a].result;
            if (fwd.src_b == ForwardSource::EX_MEM_STAGE) op2_val = ex_mem.slots[fwd.slot_b].alu_result;
            else if (fwd.src_b == ForwardSource::MEM_WB_STAGE) op2_val = mem_wb.slots[fwd.slot_b].result;
            if (fwd.src_a != ForwardSource::REGISTER_FILE || fwd.src_b != ForwardSource::REGISTER_FILE) forward_count++;
        }
        uint32_t res = 0;
        bool is_branch = (entry.dec.opcode == 0x63), is_jump = (entry.dec.opcode == 0x6F || entry.dec.opcode == 0x67);
        bool actually_taken = false; uint32_t actual_target = 0;
        if (is_jump) {
            actually_taken = true;
            actual_target = (entry.dec.opcode == 0x6F) ? entry.pc + entry.dec.imm : (op1 + entry.dec.imm) & ~1;
            res = entry.pc + 4;
        } else if (is_branch) {
            switch (entry.dec.funct3) {
                case 0: actually_taken = (op1 == op2_val); break;
                case 1: actually_taken = (op1 != op2_val); break;
                case 4: actually_taken = ((int32_t)op1 < (int32_t)op2_val); break;
                case 5: actually_taken = ((int32_t)op1 >= (int32_t)op2_val); break;
                case 6: actually_taken = (op1 < op2_val); break;
                case 7: actually_taken = (op1 >= op2_val); break;
            }
            actual_target = entry.pc + entry.dec.imm;
        } else if (entry.dec.opcode == 0x37) res = entry.dec.imm;
        else if (entry.dec.opcode == 0x17) res = entry.pc + entry.dec.imm;
        else {
            uint32_t alu_op2 = (entry.dec.type == InstructionType::I_TYPE || entry.dec.type == InstructionType::S_TYPE) ? entry.dec.imm : op2_val;
            res = compute_alu(entry.dec, op1, alu_op2);
        }
        next_ex_mem.slots[i] = {true, entry.pc, entry.dec.rd, res, op2_val, entry.dec};
        next_ex_mem.count++;
        if (is_branch || is_jump) {
            predictor.update(entry.pc, actual_target, actually_taken);
            if ((actually_taken != entry.pred_taken) || (actually_taken && actual_target != entry.pred_target)) {
                branch_mispredicted = true; correct_target_pc = actually_taken ? actual_target : entry.pc + 4;
                break;
            }
        }
    }
}

void Pipeline::stage_memory() {
    if (ex_mem.count == 0) return;
    for (uint32_t i = 0; i < ex_mem.count; ++i) {
        const auto& entry = ex_mem.slots[i];
        if (!entry.valid) continue;
        if (config.dcache.enabled && (entry.dec.opcode == 0x03 || entry.dec.opcode == 0x23)) {
            uint32_t p = dcache.access(entry.alu_result, cycle_count);
            if (p > 0) { remaining_cache_stall = p; return; }
        }
        uint32_t res = entry.alu_result;
        if (entry.dec.opcode == 0x03) {
            uint32_t addr = entry.alu_result;
            switch (entry.dec.funct3) {
                case 0: res = (int32_t)(int8_t)memory.read_byte(addr); break;
                case 1: res = (int32_t)(int16_t)memory.read_half(addr); break;
                case 2: res = memory.read_word(addr); break;
                case 4: res = memory.read_byte(addr); break;
                case 5: res = memory.read_half(addr); break;
            }
        } else if (entry.dec.opcode == 0x23) {
            uint32_t addr = entry.alu_result, val = entry.rs2_val;
            switch (entry.dec.funct3) {
                case 0: memory.write_byte(addr, val & 0xFF); break;
                case 1: memory.write_half(addr, val & 0xFFFF); break;
                case 2: memory.write_word(addr, val); break;
            }
        }
        next_mem_wb.slots[i] = {true, entry.pc, entry.rd, res, entry.dec};
        next_mem_wb.count++;
    }
}

void Pipeline::stage_writeback() {
    if (mem_wb.count == 0) return;
    for (uint32_t i = 0; i < mem_wb.count; ++i) {
        const auto& entry = mem_wb.slots[i];
        if (!entry.valid) continue;
        bool writes = (entry.dec.type == InstructionType::R_TYPE || entry.dec.type == InstructionType::I_TYPE || 
                       entry.dec.type == InstructionType::U_TYPE || entry.dec.type == InstructionType::J_TYPE);
        if (writes && entry.rd != 0) rf.write(entry.rd, entry.result);
        inst_count++;
    }
}

uint32_t Pipeline::compute_alu(const DecodedInstruction& dec, uint32_t op1, uint32_t op2) {
    if (dec.opcode == 0x33) {
        if (dec.funct7 == 0x01) {
            switch (dec.funct3) {
                case 0: return (uint32_t)((int32_t)op1 * (int32_t)op2);
                case 1: return (uint32_t)(((int64_t)(int32_t)op1 * (int64_t)(int32_t)op2) >> 32);
                case 2: return (uint32_t)(((int64_t)(int32_t)op1 * (uint64_t)op2) >> 32);
                case 3: return (uint32_t)(((uint64_t)op1 * (uint64_t)op2) >> 32);
                case 4: return (op2 == 0) ? 0xFFFFFFFF : (int32_t)op1 / (int32_t)op2;
                case 5: return (op2 == 0) ? 0xFFFFFFFF : op1 / op2;
                case 6: return (op2 == 0) ? op1 : (int32_t)op1 % (int32_t)op2;
                case 7: return (op2 == 0) ? op1 : op1 % op2;
            }
        } else {
            switch (dec.funct3) {
                case 0: return (dec.funct7 == 0x00) ? op1 + op2 : op1 - op2;
                case 1: return op1 << (op2 & 0x1F);
                case 2: return ((int32_t)op1 < (int32_t)op2) ? 1 : 0;
                case 3: return (op1 < op2) ? 1 : 0;
                case 4: return op1 ^ op2;
                case 5: return (dec.funct7 == 0x00) ? op1 >> (op2 & 0x1F) : (int32_t)op1 >> (op2 & 0x1F);
                case 6: return op1 | op2;
                case 7: return op1 & op2;
            }
        }
    } else if (dec.opcode == 0x13) {
        switch (dec.funct3) {
            case 0: return op1 + op2;
            case 2: return ((int32_t)op1 < (int32_t)op2) ? 1 : 0;
            case 3: return (op1 < (uint32_t)op2) ? 1 : 0;
            case 4: return op1 ^ op2;
            case 6: return op1 | op2;
            case 7: return op1 & op2;
            case 1: return op1 << (op2 & 0x1F);
            case 5: return ((dec.raw >> 30) == 0) ? op1 >> (op2 & 0x1F) : (int32_t)op1 >> (op2 & 0x1F);
        }
    } else if (dec.opcode == 0x23 || dec.opcode == 0x03) return op1 + op2;
    return 0;
}

void Pipeline::print_pipeline_diagram() const {
    auto pc_str = [](const auto& latch, int slot) {
        if (slot >= (int)MAX_WIDTH || !latch.slots[slot].valid) return std::string(" [---] ");
        return " 0x" + Memory::to_hex_string(latch.slots[slot].pc) + " ";
    };
    std::stringstream ss;
    ss << "Cycle " << std::setw(4) << cycle_count << " | IF:" << pc_str(if_id, 0) << pc_str(if_id, 1) 
       << "| ID:" << pc_str(id_ex, 0) << pc_str(id_ex, 1) << "| EX:" << pc_str(ex_mem, 0) << pc_str(ex_mem, 1) 
       << "| WB:" << pc_str(mem_wb, 0) << pc_str(mem_wb, 1);
    if (remaining_cache_stall > 0) ss << " << CACHE";
    else if (hazard_signals.stall_if) ss << " << STALL";
    LOG_INFO(ss.str());
}

ExecutionMetrics Pipeline::get_metrics() const {
    ExecutionMetrics m; m.config_name = config.name; m.cycles = cycle_count; m.instructions = inst_count;
    m.stalls_hazard = stall_count; m.stalls_cache = cache_stall_count; m.flushes = flush_count;
    m.forwarding_events = forward_count; m.ipc = (cycle_count == 0) ? 0 : (double)inst_count / cycle_count;
    m.icache_hit_rate = icache.get_stats().hit_rate(); m.dcache_hit_rate = dcache.get_stats().hit_rate();
    return m;
}

void Pipeline::dump_state() const {
    ExecutionMetrics m = get_metrics();
    LOG_INFO("--- Pipeline State [" + m.config_name + "] ---");
    LOG_INFO("Width: " + std::to_string(config.issue_width) + " | Cycles: " + std::to_string(m.cycles) + " | Instr: " + std::to_string(m.instructions) + " | IPC: " + std::to_string(m.ipc));
    rf.dump();
}

} // namespace phlego
