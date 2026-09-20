#include "cache/cache.h"
#include <cmath>
#include <algorithm>

namespace phlego {

static uint32_t log2_uint(uint32_t n) {
    uint32_t res = 0;
    while (n >>= 1) res++;
    return res;
}

Cache::Cache(const Config& cfg) {
    this->config = cfg;
    
    // Safety check to avoid bad_alloc if size is 0 or invalid
    if (config.size_bytes == 0 || config.associativity == 0 || config.line_size_bytes == 0) {
        num_sets = 0;
        set_index_mask = 0;
        line_offset_bits = 0;
        set_index_bits = 0;
        return;
    }

    num_sets = config.size_bytes / (config.line_size_bytes * config.associativity);
    line_offset_bits = log2_uint(config.line_size_bytes);
    set_index_bits = log2_uint(num_sets);
    set_index_mask = (num_sets > 0) ? (num_sets - 1) : 0;

    sets.resize(num_sets);
    for (auto& set : sets) {
        set.lines.resize(config.associativity);
    }
}

uint32_t Cache::access(uint32_t address, uint64_t current_cycle) {
    if (num_sets == 0) return 0; // Cache disabled/invalid
    stats.total_accesses++;

    uint32_t set_index = (address >> line_offset_bits) & set_index_mask;
    uint32_t tag = address >> (line_offset_bits + set_index_bits);

    CacheSet& set = sets[set_index];
    for (auto& line : set.lines) {
        if (line.valid && line.tag == tag) {
            line.last_access = current_cycle;
            stats.hits++;
            return 0;
        }
    }

    stats.misses++;
    CacheLine* lru_line = &set.lines[0];
    for (auto& line : set.lines) {
        if (!line.valid) { lru_line = &line; break; }
        if (line.last_access < lru_line->last_access) lru_line = &line;
    }

    lru_line->valid = true;
    lru_line->tag = tag;
    lru_line->last_access = current_cycle;
    return config.miss_penalty;
}

} // namespace phlego
