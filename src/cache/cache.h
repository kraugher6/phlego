#ifndef CACHE_H
#define CACHE_H

#include <cstdint>
#include <vector>
#include <string>

namespace phlego {

struct CacheLine {
    bool valid = false;
    uint32_t tag = 0;
    uint64_t last_access = 0; // For LRU
};

struct CacheSet {
    std::vector<CacheLine> lines;
};

class Cache {
public:
    struct Config {
        std::string name;
        uint32_t size_bytes;
        uint32_t associativity;
        uint32_t line_size_bytes;
        uint32_t miss_penalty;
    };

    Cache(const Config& config);

    // Returns number of stall cycles incurred (0 for hit, miss_penalty for miss)
    uint32_t access(uint32_t address, uint64_t current_cycle);

    struct Stats {
        uint64_t hits = 0;
        uint64_t misses = 0;
        uint64_t total_accesses = 0;
        double hit_rate() const { 
            return total_accesses == 0 ? 0 : (double)hits / total_accesses; 
        }
    };

    const Stats& get_stats() const { return stats; }
    const Config& get_config() const { return config; }

private:
    Config config;
    uint32_t num_sets;
    uint32_t set_index_mask;
    uint32_t line_offset_bits;
    uint32_t set_index_bits;

    std::vector<CacheSet> sets;
    Stats stats;
};

} // namespace phlego

#endif
