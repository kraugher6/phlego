#include "prediction/branch_predictor.h"

namespace phlego {

BranchPredictor::BranchPredictor(uint32_t btb_size_val) : btb_size(btb_size_val) {
    bht.resize(btb_size, 1); // Initialize all to 01 (Weakly Not Taken)
}

Prediction BranchPredictor::predict(uint32_t pc) {
    Prediction pred;
    uint32_t idx = get_index(pc);
    
    // Prediction based on 2-bit counter
    // Bit 1 determines taken/not-taken
    pred.taken = (bht[idx] >= 2);

    // If predicted taken, look up target in BTB
    if (pred.taken) {
        auto it = btb.find(pc);
        if (it != btb.end()) {
            pred.target_pc = it->second;
        } else {
            // We predicted taken but don't have a target yet!
            // This happens for new branches. Fallback to not-taken.
            pred.taken = false;
        }
    }

    return pred;
}

void BranchPredictor::update(uint32_t pc, uint32_t target_pc, bool actually_taken) {
    uint32_t idx = get_index(pc);
    uint8_t state = bht[idx];

    // Update 2-bit saturating counter
    if (actually_taken) {
        if (state < 3) bht[idx]++;
        btb[pc] = target_pc; // Learn/update target
    } else {
        if (state > 0) bht[idx]--;
    }

    // Statistics (we'll update these from the pipeline when we know if we were right)
}

} // namespace phlego
