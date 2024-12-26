# Variabili di configurazione
RISCV_GCC = riscv64-unknown-elf-gcc
GCC_FLAGS = -march=rv32i -mabi=ilp32 -nostartfiles -I/usr/lib/gcc/riscv64-unknown-elf/10.2.0/include
SRC_DIR = src
BUILD_DIR = build
BIN_DIR = bin
TEST_DIR = test
TEST_SRC = $(TEST_DIR)/simple_program.cpp
TEST_OUT_S = $(TEST_DIR)/simple_program.s
TEST_OUT_BIN = $(TEST_DIR)/simple_program.bin
TEST_OUT_MAP = $(TEST_DIR)/simple_program.map
TEXT_SECTION_DIS = $(TEST_DIR)/simple_program.dis

# Set the logging level (DEBUG, INFO, ERROR)
LOG_LEVEL ?= LOG_LEVEL_DEBUG

# Obiettivi principali
all: $(BIN_DIR)/emulator

# Compila l'emulatore
$(BIN_DIR)/emulator: $(BUILD_DIR)/cpu.o $(BUILD_DIR)/memory.o $(BUILD_DIR)/main.o | $(BIN_DIR)
	g++ -o $(BIN_DIR)/emulator $(BUILD_DIR)/cpu.o $(BUILD_DIR)/memory.o $(BUILD_DIR)/main.o

# Regole per compilare i file sorgenti
$(BUILD_DIR)/cpu.o: $(SRC_DIR)/cpu.cpp $(SRC_DIR)/cpu.h | $(BUILD_DIR)
	g++ -Wall -Wextra -std=c++17 -DLOG_LEVEL=$(LOG_LEVEL) -c $(SRC_DIR)/cpu.cpp -o $(BUILD_DIR)/cpu.o

$(BUILD_DIR)/memory.o: $(SRC_DIR)/memory.cpp $(SRC_DIR)/memory.h | $(BUILD_DIR)
	g++ -Wall -Wextra -std=c++17 -DLOG_LEVEL=$(LOG_LEVEL) -c $(SRC_DIR)/memory.cpp -o $(BUILD_DIR)/memory.o

$(BUILD_DIR)/main.o: $(SRC_DIR)/main.cpp $(SRC_DIR)/cpu.h $(SRC_DIR)/memory.h | $(BUILD_DIR)
	g++ -Wall -Wextra -std=c++17 -DLOG_LEVEL=$(LOG_LEVEL) -c $(SRC_DIR)/main.cpp -o $(BUILD_DIR)/main.o

# Regola per generare il file .s da un programma di test
tests: $(TEST_SRC)
	$(RISCV_GCC) $(GCC_FLAGS) -S $(TEST_SRC) -o $(TEST_OUT_S)
	$(RISCV_GCC) $(GCC_FLAGS) -Wl,-Map=$(TEST_OUT_MAP) -o $(TEST_OUT_BIN) $(TEST_SRC)
	riscv64-unknown-elf-objdump -D --section=.text $(TEST_OUT_BIN) > $(TEXT_SECTION_DIS)

# Directory necessarie
$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

$(BIN_DIR):
	mkdir -p $(BIN_DIR)

# Pulizia dei file generati
clean:
	rm -rf $(BUILD_DIR) $(BIN_DIR) $(TEST_DIR)/*.s $(TEST_DIR)/*.bin $(TEST_DIR)/*.dis $(TEST_DIR)/*.map
