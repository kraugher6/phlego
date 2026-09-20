#include <gtest/gtest.h>
#include "hazard/hazard_unit.h"
#include "core/decoder.h"
#include "core/pipeline_regs.h"

using namespace phlego;

TEST(HazardUnitTest, NoHazard) {
    DecodedInstruction id_dec;
    id_dec.type = InstructionType::R_TYPE;
    id_dec.rs1 = 1;
    id_dec.rs2 = 2;
    id_dec.rd = 3;

    ID_EX id_ex;
    EX_MEM ex_mem;
    MEM_WB mem_wb;

    HazardSignals signals = HazardUnit::detect(id_dec, id_ex, ex_mem, mem_wb, false);
    EXPECT_FALSE(signals.stall_id);
    EXPECT_FALSE(signals.stall_if);
}

TEST(HazardUnitTest, RawHazardWithoutForwarding) {
    DecodedInstruction id_dec;
    id_dec.type = InstructionType::R_TYPE;
    id_dec.rs1 = 1;
    id_dec.rs2 = 2;
    id_dec.rd = 3;

    ID_EX id_ex;
    id_ex.slots[0].valid = true;
    id_ex.slots[0].dec.type = InstructionType::I_TYPE;
    id_ex.slots[0].dec.rd = 1;

    EX_MEM ex_mem;
    MEM_WB mem_wb;

    HazardSignals signals = HazardUnit::detect(id_dec, id_ex, ex_mem, mem_wb, false);
    EXPECT_TRUE(signals.stall_id);
    EXPECT_TRUE(signals.stall_if);
    EXPECT_TRUE(signals.flush_ex);

    HazardSignals signals_fwd = HazardUnit::detect(id_dec, id_ex, ex_mem, mem_wb, true);
    EXPECT_FALSE(signals_fwd.stall_id);
}

TEST(HazardUnitTest, LoadUseHazard) {
    DecodedInstruction id_dec;
    id_dec.type = InstructionType::R_TYPE;
    id_dec.rs1 = 1;

    ID_EX id_ex;
    id_ex.slots[0].valid = true;
    id_ex.slots[0].dec.opcode = 0x03; // LOAD
    id_ex.slots[0].dec.rd = 1;

    EX_MEM ex_mem;
    MEM_WB mem_wb;

    HazardSignals signals = HazardUnit::detect(id_dec, id_ex, ex_mem, mem_wb, true);
    EXPECT_TRUE(signals.stall_id);
    EXPECT_TRUE(signals.stall_if);
    EXPECT_TRUE(signals.flush_ex);
}
