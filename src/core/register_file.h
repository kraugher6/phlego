#ifndef REGISTER_FILE_H
#define REGISTER_FILE_H

#include <array>
#include <cstdint>
#include <string>

namespace phlego {

class RegisterFile {
public:
    RegisterFile();

    uint32_t read(uint8_t reg) const;
    void write(uint8_t reg, uint32_t val);

    std::array<uint32_t, 32> get_all_registers() const { return registers; }

    void dump() const;

private:
    std::array<uint32_t, 32> registers;
    static const std::array<std::string, 32> reg_names;
};

} // namespace phlego

#endif
