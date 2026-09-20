# Compiler and flags
RISCV_GCC = riscv64-unknown-elf-gcc
GCC_FLAGS = -march=rv32i -mabi=ilp32 -nostartfiles
RISCV_OBJDUMP = riscv64-unknown-elf-objdump
OBJDUMP_FLAGS = -D --section=.text

# Test source and output files
TEST_SRC = rv32i_stall.cpp
TEST_OUT_S = rv32i_stall.s
TEST_OUT_BIN = rv32i_stall.bin
TEST_OUT_MAP = rv32i_stall.map
TEXT_SECTION_DIS = rv32i_stall.dis

# Regola per generare il file .s da un programma di test
all: $(TEST_OUT_S) $(TEST_OUT_BIN) $(TEXT_SECTION_DIS)

$(TEST_OUT_S): $(TEST_SRC)
	$(RISCV_GCC) $(GCC_FLAGS) -S $(TEST_SRC) -o $(TEST_OUT_S)

$(TEST_OUT_BIN): $(TEST_SRC)
	$(RISCV_GCC) $(GCC_FLAGS) -Wl,-Map=$(TEST_OUT_MAP) -o $(TEST_OUT_BIN) $(TEST_SRC)

$(TEXT_SECTION_DIS): $(TEST_OUT_BIN)
	$(RISCV_OBJDUMP) $(OBJDUMP_FLAGS) $(TEST_OUT_BIN) > $(TEXT_SECTION_DIS)

# Pulizia dei file generati
clean:
	rm -f $(TEST_OUT_S) $(TEST_OUT_BIN) $(TEST_OUT_MAP) $(TEXT_SECTION_DIS)