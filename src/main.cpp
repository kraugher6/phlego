#include <iostream>
#include "core/memory.h"
#include "core/pipeline.h"
#include "core/runner.h"
#include "logger.h"

using namespace phlego;

int main(int argc, char* argv[]) {
    if (argc < 2) {
        std::cerr << "Usage: " << argv[0] << " <elf_file>" << std::endl;
        return 1;
    }

    std::string elf_file = argv[1];

    Memory memory;
    uint32_t entry_point = 0;
    if (!memory.load_elf(elf_file, entry_point)) {
        LOG_ERROR("Failed to load ELF file");
        return 1;
    }

    Pipeline pipeline(memory);
    pipeline.set_pc(entry_point);
    pipeline.set_register(2, 0x07FFFFF0); // Default SP

    Runner runner(pipeline);
    runner.run();

    return 0;
}