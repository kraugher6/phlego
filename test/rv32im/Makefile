# Compiler and flags
RISCV_GCC = riscv64-unknown-elf-gcc
GCC_FLAGS = -march=rv32im -mabi=ilp32 -nostartfiles
RISCV_OBJDUMP = riscv64-unknown-elf-objdump
OBJDUMP_FLAGS = -D --section=.text

# Test source and output files
TEST_SRC = rv32im.cpp
TEST_OUT_S = rv32im.s
TEST_OUT_BIN = rv32im.bin
TEST_OUT_MAP = rv32im.map
TEXT_SECTION_DIS = rv32im.dis

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