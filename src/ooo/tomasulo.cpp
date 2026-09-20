#include "ooo/tomasulo.h"
#include "logger.h"
#include <iostream>
#include <iomanip>
#include <sstream>

namespace phlego {

TomasuloUnit::TomasuloUnit(uint32_t rob_size) {
    rob.resize(rob_size);
    for (int i = 0; i < 32; ++i) rat[i] = -1;

    // Define Reservation Stations
    rs_list.push_back({"Int1"});
    rs_list.push_back({"Int2"});
    rs_list.push_back({"Mult1"});
    rs_list.push_back({"Mult2"});
    rs_list.push_back({"Load1"});
    rs_list.push_back({"Store1"});
}

uint32_t TomasuloUnit::get_latency(const DecodedInstruction& dec) {
    if (dec.opcode == 0x33 && dec.funct7 == 0x01) return 4; // Multiply latency
    if (dec.opcode == 0x03 || dec.opcode == 0x23) return 2; // Memory latency
    return 1; // Basic ALU latency
}

ReservationStation* TomasuloUnit::find_free_rs(const DecodedInstruction& dec) {
    std::string prefix = "Int";
    if (dec.opcode == 0x33 && dec.funct7 == 0x01) prefix = "Mult";
    else if (dec.opcode == 0x03) prefix = "Load";
    else if (dec.opcode == 0x23) prefix = "Store";

    for (auto& rs : rs_list) {
        if (!rs.busy && rs.name.find(prefix) == 0) return &rs;
    }
    return nullptr;
}

bool TomasuloUnit::can_issue() const {
    return rob_count < rob.size();
}

void TomasuloUnit::issue(const DecodedInstruction& dec, uint32_t pc, const std::array<uint32_t, 32>& rf_vals) {
    ReservationStation* rs = find_free_rs(dec);
    if (!rs || !can_issue()) return;

    // 1. Allocate ROB entry
    uint32_t rob_idx = rob_tail;
    rob[rob_idx].valid = true;
    rob[rob_idx].ready = false;
    rob[rob_idx].pc = pc;
    rob[rob_idx].dest_reg = dec.rd;
    rob[rob_idx].dec = dec;
    
    rob_tail = (rob_tail + 1) % rob.size();
    rob_count++;

    // 2. Allocate RS
    rs->busy = true;
    rs->pc = pc;
    rs->op = dec.opcode;
    rs->dest_rob_idx = rob_idx;
    rs->remaining_cycles = get_latency(dec);

    // Operand rs1
    if (dec.rs1 == 0) { rs->vj = 0; rs->qj = -1; }
    else if (rat[dec.rs1] != -1) {
        uint32_t producer_rob = rat[dec.rs1];
        if (rob[producer_rob].ready) {
            rs->vj = rob[producer_rob].value;
            rs->qj = -1;
        } else {
            rs->qj = producer_rob;
        }
    } else {
        rs->vj = rf_vals[dec.rs1];
        rs->qj = -1;
    }

    // Operand rs2
    if (dec.rs2 == 0) { rs->vk = 0; rs->qk = -1; }
    else if (rat[dec.rs2] != -1) {
        uint32_t producer_rob = rat[dec.rs2];
        if (rob[producer_rob].ready) {
            rs->vk = rob[producer_rob].value;
            rs->qk = -1;
        } else {
            rs->qk = producer_rob;
        }
    } else {
        rs->vk = rf_vals[dec.rs2];
        rs->qk = -1;
    }

    // 3. Update RAT
    if (dec.rd != 0) {
        rat[dec.rd] = rob_idx;
    }
}

void TomasuloUnit::execute_tick() {
    for (auto& rs : rs_list) {
        if (rs.busy && rs.qj == -1 && rs.qk == -1) {
            if (rs.remaining_cycles > 0) rs.remaining_cycles--;
        }
    }
}

void TomasuloUnit::write_result() {
    // CDB Broadcast
    for (auto& rs : rs_list) {
        if (rs.busy && rs.remaining_cycles == 0) {
            uint32_t result = 0; // Simplified: in reality we would compute based on RS vj/vk
            uint32_t rob_idx = rs.dest_rob_idx;

            // Broadcast to other RS
            for (auto& other_rs : rs_list) {
                if (other_rs.busy) {
                    if (other_rs.qj == (int32_t)rob_idx) { other_rs.vj = result; other_rs.qj = -1; }
                    if (other_rs.qk == (int32_t)rob_idx) { other_rs.vk = result; other_rs.qk = -1; }
                }
            }

            // Update ROB
            rob[rob_idx].value = result;
            rob[rob_idx].ready = true;

            rs.clear();
        }
    }
}

TomasuloUnit::CommitInfo TomasuloUnit::commit() {
    CommitInfo info;
    if (rob_count > 0 && rob[rob_head].ready) {
        info.valid = true;
        info.rd = rob[rob_head].dest_reg;
        info.value = rob[rob_head].value;
        info.pc = rob[rob_head].pc;

        // Clear RAT if we were the last one writing to this register
        if (info.rd != 0 && rat[info.rd] == (int32_t)rob_head) {
            rat[info.rd] = -1;
        }

        rob[rob_head].clear();
        rob_head = (rob_head + 1) % rob.size();
        rob_count--;
    }
    return info;
}

void TomasuloUnit::dump_state() const {
    LOG_INFO("--- Tomasulo/ROB State ---");
    LOG_INFO("RS Name\tBusy\tQj\tQk\tDestROB\tPC");
    for (const auto& rs : rs_list) {
        std::stringstream ss;
        ss << rs.name << "\t" << (rs.busy ? "Y":"N") << "\t" 
           << rs.qj << "\t" << rs.qk << "\t" << rs.dest_rob_idx << "\t" << std::hex << rs.pc;
        LOG_INFO(ss.str());
    }
    LOG_INFO("ROB Head: " + std::to_string(rob_head) + " Tail: " + std::to_string(rob_tail) + " Count: " + std::to_string(rob_count));
}

} // namespace phlego
