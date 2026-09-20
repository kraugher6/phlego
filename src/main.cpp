#include <iostream>
#include <vector>
#include <iomanip>
#include "core/memory.h"
#include "core/pipeline.h"
#include "core/runner.h"
#include "logger.h"

using namespace phlego;

void print_comparison(const std::vector<ExecutionMetrics>& all_metrics) {
    std::cout << "\n" << std::string(100, '=') << "\n";
    std::cout << std::left << std::setw(25) << "Configuration" 
              << std::setw(10) << "Cycles" 
              << std::setw(10) << "Instr" 
              << std::setw(10) << "IPC" 
              << std::setw(10) << "Stalls" 
              << std::setw(10) << "Flushes" 
              << std::setw(10) << "FWD" << "\n";
    std::cout << std::string(100, '-') << "\n";

    for (const auto& m : all_metrics) {
        std::cout << std::left << std::setw(25) << m.config_name
                  << std::setw(10) << m.cycles
                  << std::setw(10) << m.instructions
                  << std::setw(10) << std::fixed << std::setprecision(3) << m.ipc
                  << std::setw(10) << (m.stalls_hazard + m.stalls_cache)
                  << std::setw(10) << m.flushes
                  << std::setw(10) << m.forwarding_events << "\n";
    }
    std::cout << std::string(100, '=') << "\n\n";
}

int main(int argc, char* argv[]) {
    if (argc < 2) {
        std::cerr << "Usage: " << argv[0] << " <elf_file> [--compare]" << std::endl;
        return 1;
    }

    std::string elf_file = argv[1];
    bool compare_mode = (argc > 2 && std::string(argv[2]) == "--compare");

    std::vector<SimConfig> configs;
    if (compare_mode) {
        SimConfig c1 = SimConfig::baseline();
        c1.name = "Stalls Only (No BPU)";
        configs.push_back(c1);

        SimConfig c2;
        c2.name = "Hazards (Stalls Only)";
        c2.data_forwarding = false;
        c2.icache.enabled = false;
        c2.dcache.enabled = false;
        configs.push_back(c2);

        SimConfig c3;
        c3.name = "Full Pipeline (FWD)";
        c3.icache.enabled = false;
        c3.dcache.enabled = false;
        configs.push_back(c3);

        SimConfig c4;
        c4.name = "Realistic (FWD + Cache)";
        configs.push_back(c4);

        SimConfig c5;
        c5.name = "Superscalar (2-wide)";
        c5.issue_width = 2;
        configs.push_back(c5);

        SimConfig c6;
        c6.name = "Scoreboard (OoO)";
        c6.pipeline_type = PipelineType::SCOREBOARD;
        configs.push_back(c6);

        SimConfig c7;
        c7.name = "Tomasulo (OoO + ROB)";
        c7.pipeline_type = PipelineType::TOMASULO;
        configs.push_back(c7);
    } else {
        configs.push_back(SimConfig());
    }

    std::vector<ExecutionMetrics> all_metrics;

    for (const auto& cfg : configs) {
        LOG_INFO(">>> Running simulation: " + cfg.name);
        
        Memory memory;
        uint32_t entry_point = 0;
        if (!memory.load_elf(elf_file, entry_point)) {
            LOG_ERROR("Failed to load ELF file");
            continue;
        }

        Pipeline pipeline(memory, cfg);
        pipeline.enable_tracing("trace_" + cfg.name + ".json");
        pipeline.set_pc(entry_point);
        pipeline.set_register(2, 0x07FFFFF0);

        Runner runner(pipeline);
        runner.run();
        
        all_metrics.push_back(pipeline.get_metrics());
    }

    if (compare_mode) {
        print_comparison(all_metrics);
    }

    return 0;
}