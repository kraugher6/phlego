#include <gtest/gtest.h>
#include "core/memory.h"
#include <stdexcept>

using namespace phlego;

TEST(MemoryTest, StoreLoadWord) {
    Memory mem(64);
    mem.write_word(0, 0x12345678);
    EXPECT_EQ(mem.read_byte(0), 0x78); // Little endian
    EXPECT_EQ(mem.read_half(0), 0x5678);
    EXPECT_EQ(mem.read_word(0), 0x12345678u);
}

TEST(MemoryTest, StoreLoadByteHalf) {
    Memory mem(8);
    mem.write_byte(2, 0xAB);
    mem.write_half(3, 0xCDEF);
    EXPECT_EQ(mem.read_byte(2), 0xAB);
    EXPECT_EQ(mem.read_half(3), 0xCDEF);
}

TEST(MemoryTest, OutOfRangeReturnsZero) {
    Memory mem(4);
    mem.write_word(2, 0xdeadbeef); // Out of bounds for word at 2 (needs 4 bytes -> 2+4=6 > 4)
    EXPECT_EQ(mem.read_word(1), 0u);
}
