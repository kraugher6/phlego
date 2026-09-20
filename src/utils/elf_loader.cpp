#include "elf_loader.h"
#include <fstream>
#include <iostream>
#include <cstring>
#include "logger.h"

namespace phlego {

ElfLoader::ElfLoader(const std::string& fname) : filename(fname), entry_point(0) {}

bool ElfLoader::load() {
    std::ifstream file(filename, std::ios::binary);
    if (!file.is_open()) {
        LOG_ERROR("Could not open ELF file: " + filename);
        return false;
    }

    Elf32_Ehdr header;
    file.read(reinterpret_cast<char*>(&header), sizeof(Elf32_Ehdr));

    if (!validate_header(header)) {
        return false;
    }

    entry_point = header.e_entry;

    // Seek to program header table
    file.seekg(header.e_phoff);

    for (int i = 0; i < header.e_phnum; ++i) {
        Elf32_Phdr phdr;
        file.read(reinterpret_cast<char*>(&phdr), sizeof(Elf32_Phdr));

        if (phdr.p_type == PT_LOAD) {
            ElfSegment segment;
            segment.vaddr = phdr.p_vaddr;
            segment.memsz = phdr.p_memsz;
            segment.flags = phdr.p_flags;
            segment.data.resize(phdr.p_filesz);

            // Save current position and jump to segment offset
            std::streampos current_pos = file.tellg();
            file.seekg(phdr.p_offset);
            file.read(reinterpret_cast<char*>(segment.data.data()), phdr.p_filesz);
            file.seekg(current_pos);

            segments.push_back(std::move(segment));
            LOG_DEBUG("Loaded ELF segment at vaddr 0x" + std::to_string(phdr.p_vaddr) + ", size 0x" + std::to_string(phdr.p_memsz));
        }
    }

    return true;
}

bool ElfLoader::validate_header(const Elf32_Ehdr& header) {
    // Magic number: 0x7F 'E' 'L' 'F'
    if (header.e_ident[0] != 0x7F || header.e_ident[1] != 'E' ||
        header.e_ident[2] != 'L' || header.e_ident[3] != 'F') {
        LOG_ERROR("Not a valid ELF file");
        return false;
    }

    // 32-bit check (ELFCLASS32 = 1)
    if (header.e_ident[4] != 1) {
        LOG_ERROR("Only 32-bit ELF files are supported");
        return false;
    }

    // Machine check (EM_RISCV = 243)
    if (header.e_machine != 243) {
        LOG_ERROR("Not a RISC-V ELF file (Machine code: " + std::to_string(header.e_machine) + ")");
        return false;
    }

    return true;
}

} // namespace phlego
