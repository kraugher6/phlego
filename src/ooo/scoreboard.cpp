#include "ooo/scoreboard.h"
#include "logger.h"
#include <iostream>
#include <iomanip>

namespace phlego {

Scoreboard::Scoreboard() {
    fus.push_back({"Integer", FUType::INTEGER});
    fus.push_back({"Mult1", FUType::MULTIPLIER});
    fus.push_back({"Mult2", FUType::MULTIPLIER});
    fus.push_back({"Divider", FUType::DIVIDER});
    fus.push_back({"Memory", FUType::MEMORY});

    for (int i = 0; i < 32; ++i) register_result[i] = "";
}

FUType Scoreboard::get_required_fu_type(const DecodedInstruction& dec) {
    if (dec.opcode == 0x33 && dec.funct7 == 0x01) {
        if (dec.funct3 <= 3) return FUType::MULTIPLIER;
        return FUType::DIVIDER;
    }
    if (dec.opcode == 0x03 || dec.opcode == 0x23) return FUType::MEMORY;
    return FUType::INTEGER;
}

FunctionalUnit* Scoreboard::get_fu(const std::string& name) {
    for (auto& fu : fus) if (fu.name == name) return &fu;
    return nullptr;
}

bool Scoreboard::can_issue(const DecodedInstruction& dec) {
    FUType type = get_required_fu_type(dec);
    
    // Check Structural Hazard (is any FU of this type free?)
    bool fu_free = false;
    for (const auto& fu : fus) {
        if (fu.type == type && !fu.busy) { fu_free = true; break; }
    }
    if (!fu_free) return false;

    // Check WAW Hazard (is any FU writing to the same destination?)
    if (dec.rd != 0 && register_result[dec.rd] != "") return false;

    return true;
}

void Scoreboard::issue(const DecodedInstruction& dec, uint32_t pc) {
    FUType type = get_required_fu_type(dec);
    FunctionalUnit* target_fu = nullptr;
    for (auto& fu : fus) {
        if (fu.type == type && !fu.busy) { target_fu = &fu; break; }
    }

    target_fu->busy = true;
    target_fu->pc = pc;
    target_fu->op = dec.opcode;
    target_fu->fi = dec.rd;
    target_fu->fj = dec.rs1;
    target_fu->fk = dec.rs2;

    // Set Qj, Qk (who is producing our sources?)
    if (dec.rs1 != 0) target_fu->qj = register_result[dec.rs1];
    if (dec.rs2 != 0) target_fu->qk = register_result[dec.rs2];

    // Set Rj, Rk (are sources ready?)
    target_fu->rj = (target_fu->qj == "");
    target_fu->rk = (target_fu->qk == "");

    // Book destination register
    if (dec.rd != 0) register_result[dec.rd] = target_fu->name;

    // Set latency
    switch(type) {
        case FUType::INTEGER: target_fu->remaining_cycles = 1; break;
        case FUType::MEMORY:  target_fu->remaining_cycles = 2; break;
        case FUType::MULTIPLIER: target_fu->remaining_cycles = 4; break;
        case FUType::DIVIDER: target_fu->remaining_cycles = 10; break;
    }
}

std::vector<std::string> Scoreboard::get_ready_fus() {
    std::vector<std::string> ready;
    for (auto& fu : fus) {
        if (fu.busy && fu.rj && fu.rk && fu.remaining_cycles > 0) {
            ready.push_back(fu.name);
        }
    }
    return ready;
}

void Scoreboard::read_operands(const std::string& fu_name) {
    // Logic: mark operands as read (in standard scoreboard Rj/Rk become false)
    FunctionalUnit* fu = get_fu(fu_name);
    fu->rj = false;
    fu->rk = false;
}

void Scoreboard::tick() {
    for (auto& fu : fus) {
        if (fu.busy && !fu.rj && !fu.rk && fu.remaining_cycles > 0) {
            fu.remaining_cycles--;
        }
    }
}

std::vector<std::string> Scoreboard::get_finished_fus() {
    std::vector<std::string> finished;
    for (auto& fu : fus) {
        if (fu.busy && fu.remaining_cycles == 0) finished.push_back(fu.name);
    }
    return finished;
}

bool Scoreboard::can_write_result(const std::string& fu_name) {
    FunctionalUnit* current_fu = get_fu(fu_name);
    
    // WAR Hazard Check:
    // Can we write to Fi if some other FU needs to READ our Fi but hasn't yet?
    for (const auto& fu : fus) {
        if (&fu == current_fu) continue;
        if (fu.busy) {
            if (fu.fj == current_fu->fi && fu.rj) return false;
            if (fu.fk == current_fu->fi && fu.rk) return false;
        }
    }
    return true;
}

void Scoreboard::write_result(const std::string& fu_name) {
    FunctionalUnit* finished_fu = get_fu(fu_name);
    
    // Broadcast to all waiting FUs
    for (auto& fu : fus) {
        if (fu.busy) {
            if (fu.qj == finished_fu->name) { fu.qj = ""; fu.rj = true; }
            if (fu.qk == finished_fu->name) { fu.qk = ""; fu.rk = true; }
        }
    }

    // Release register
    if (finished_fu->fi != -1 && register_result[finished_fu->fi] == finished_fu->name) {
        register_result[finished_fu->fi] = "";
    }

    finished_fu->clear();
}

void Scoreboard::dump_state() const {
    LOG_INFO("--- Scoreboard State ---");
    LOG_INFO("Name\tBusy\tOp\tFi\tFj\tFk\tQj\tQk\tRj\tRk");
    for (const auto& fu : fus) {
        std::stringstream ss;
        ss << fu.name << "\t" << (fu.busy ? "Y":"N") << "\t" 
           << std::hex << (int)fu.op << "\t" << std::dec << (int)fu.fi << "\t" 
           << (int)fu.fj << "\t" << (int)fu.fk << "\t" << fu.qj << "\t" << fu.qk << "\t"
           << (fu.rj ? "Y":"N") << "\t" << (fu.rk ? "Y":"N");
        LOG_INFO(ss.str());
    }
}

} // namespace phlego
