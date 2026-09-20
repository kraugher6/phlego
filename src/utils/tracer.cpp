#include "utils/tracer.h"
#include "core/memory.h"
#include <sstream>

namespace phlego {

Tracer::Tracer(const std::string& fname) : filename(fname) {}
Tracer::~Tracer() { save(); }

void Tracer::add_cycle(const CycleState& state) {
    trace.push_back(state);
}

void Tracer::save() {
    std::ofstream file(filename);
    if (!file.is_open()) return;

    file << "[\n";
    for (size_t i = 0; i < trace.size(); ++i) {
        const auto& s = trace[i];
        file << "  {\n";
        file << "    \"cycle\": " << s.cycle << ",\n";
        file << "    \"stages\": {\n";
        file << "      \"IF\": {\"pc\": \"0x" << Memory::to_hex_string(s.pc_if) << "\", \"valid\": " << (s.valid_if ? "true":"false") << "},\n";
        file << "      \"ID\": {\"pc\": \"0x" << Memory::to_hex_string(s.pc_id) << "\", \"valid\": " << (s.valid_id ? "true":"false") << "},\n";
        file << "      \"EX\": {\"pc\": \"0x" << Memory::to_hex_string(s.pc_ex) << "\", \"valid\": " << (s.valid_ex ? "true":"false") << "},\n";
        file << "      \"MEM\": {\"pc\": \"0x" << Memory::to_hex_string(s.pc_mem) << "\", \"valid\": " << (s.valid_mem ? "true":"false") << "},\n";
        file << "      \"WB\": {\"pc\": \"0x" << Memory::to_hex_string(s.pc_wb) << "\", \"valid\": " << (s.valid_wb ? "true":"false") << "}\n";
        file << "    },\n";
        file << "    \"flags\": {\"stall\": " << (s.stall ? "true":"false") << ", \"flush\": " << (s.flush ? "true":"false") << ", \"fwd\": " << (s.fwd ? "true":"false") << "},\n";
        file << "    \"registers\": [";
        for (size_t r = 0; r < s.registers.size(); ++r) {
            file << "\"0x" << Memory::to_hex_string(s.registers[r]) << "\"" << (r == s.registers.size()-1 ? "" : ", ");
        }
        file << "]\n";
        file << "  }" << (i == trace.size()-1 ? "" : ",") << "\n";
    }
    file << "]\n";
}

} // namespace phlego
