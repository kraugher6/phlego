#ifndef BRANCH_PREDICTOR_H
#define BRANCH_PREDICTOR_H

#include <cstdint>
#include <vector>
#include <unordered_map>

namespace phlego {

struct Prediction {
    bool taken = false;
    uint32_t target_pc = 0;
};

class BranchPredictor {
public:
    BranchPredictor(uint32_t btb_size = 128);

    Prediction predict(uint32_t pc);
    void update(uint32_t pc, uint32_t target_pc, bool actually_taken);

    struct Stats {
        uint64_t total_predictions = 0;
        uint64_t correct_predictions = 0;
        uint64_t mispredictions = 0;
        double accuracy() const { 
            return total_predictions == 0 ? 0 : (double)correct_predictions / total_predictions; 
        }
    };

    const Stats& get_stats() const { return stats; }

private:
    uint32_t btb_size;
    
    // 2-bit Bimodal Predictor State
    // 00: Strongly Not Taken
    // 01: Weakly Not Taken
    // 10: Weakly Taken
    // 11: Strongly Taken
    std::vector<uint8_t> bht; // Branch History Table (using 2-bit counters)
    
    // BTB: PC -> Target PC
    std::unordered_map<uint32_t, uint32_t> btb;

    Stats stats;

    uint32_t get_index(uint32_t pc) const {
        return (pc >> 2) % btb_size;
    }
};

} // namespace phlego

#endif
