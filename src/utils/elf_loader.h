#ifndef ELF_LOADER_H
#define ELF_LOADER_H

#include <string>
#include <vector>
#include <cstdint>

namespace phlego {

// ELF Header for 32-bit
struct Elf32_Ehdr {
    unsigned char e_ident[16];
    uint16_t      e_type;
    uint16_t      e_machine;
    uint32_t      e_version;
    uint32_t      e_entry;
    uint32_t      e_phoff;
    uint32_t      e_shoff;
    uint32_t      e_flags;
    uint16_t      e_ehsize;
    uint16_t      e_phentsize;
    uint16_t      e_phnum;
    uint16_t      e_shentsize;
    uint16_t      e_shnum;
    uint16_t      e_shstrndx;
};

// Program Header for 32-bit
struct Elf32_Phdr {
    uint32_t p_type;
    uint32_t p_offset;
    uint32_t p_vaddr;
    uint32_t p_paddr;
    uint32_t p_filesz;
    uint32_t p_memsz;
    uint32_t p_flags;
    uint32_t p_align;
};

// Segment types
#define PT_LOAD 1

struct ElfSegment {
    uint32_t vaddr;
    std::vector<uint8_t> data;
    uint32_t memsz;
    uint32_t flags;
};

class ElfLoader {
public:
    ElfLoader(const std::string& filename);
    bool load();

    uint32_t get_entry_point() const { return entry_point; }
    const std::vector<ElfSegment>& get_segments() const { return segments; }

private:
    std::string filename;
    uint32_t entry_point;
    std::vector<ElfSegment> segments;

    bool validate_header(const Elf32_Ehdr& header);
};

} // namespace phlego

#endif
