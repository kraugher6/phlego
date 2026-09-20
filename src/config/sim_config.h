#ifndef SIM_CONFIG_H
#define SIM_CONFIG_H

#include <string>
#include <vector>
#include <cstdint>

namespace phlego {

struct CacheConfig {
    std::string name = "Cache";
    bool enabled = true;
    uint32_t size_bytes = 16384;
    uint32_t associativity = 4;
    uint32_t line_size_bytes = 64;
    uint32_t miss_penalty = 10;
};

enum class PredictorType {
    NONE,
    STATIC_NOT_TAKEN,
    BIMODAL_2BIT
};

enum class PipelineType {
    IN_ORDER,
    SCOREBOARD,
    TOMASULO
};

struct SimConfig {
    std::string name = "default";
    PipelineType pipeline_type = PipelineType::IN_ORDER;
    
    // Pipeline features
    uint32_t issue_width = 1; // 1 for Scalar, 2 for Superscalar
    bool hazard_detection = true;
    bool data_forwarding = true;
    PredictorType branch_predictor = PredictorType::BIMODAL_2BIT;
    
    // Cache features
    CacheConfig icache;
    CacheConfig dcache;

    // Execution control
    uint64_t max_cycles = 1000000;

    static SimConfig baseline() {
        SimConfig cfg;
        cfg.name = "baseline_stalls";
        cfg.hazard_detection = true;
        cfg.data_forwarding = false;
        cfg.branch_predictor = PredictorType::NONE;
        cfg.icache.enabled = false;
        cfg.dcache.enabled = false;
        return cfg;
    }
};

} // namespace phlego

#endif
