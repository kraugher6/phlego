# Variabili di configurazione
SRC_DIR = src
BUILD_DIR = build
BIN_DIR = bin

# Set the logging level (DEBUG, INFO, ERROR)
LOG_LEVEL ?= LOG_LEVEL_INFO

# Obiettivi principali
all: $(BIN_DIR)/emulator

# Compila l'emulatore
$(BIN_DIR)/emulator: $(BUILD_DIR)/cpu.o $(BUILD_DIR)/memory.o $(BUILD_DIR)/main.o | $(BIN_DIR)
	g++ -o $(BIN_DIR)/emulator $(BUILD_DIR)/cpu.o $(BUILD_DIR)/memory.o $(BUILD_DIR)/main.o

# Regole per compilare i file sorgenti
$(BUILD_DIR)/cpu.o: $(SRC_DIR)/cpu.cpp $(SRC_DIR)/cpu.h | $(BUILD_DIR)
	g++ -I/home/kraugher/Documents/officina/ELFIO -Wall -Wextra -std=c++17 -DLOG_LEVEL=$(LOG_LEVEL) -c $(SRC_DIR)/cpu.cpp -o $(BUILD_DIR)/cpu.o

$(BUILD_DIR)/memory.o: $(SRC_DIR)/memory.cpp $(SRC_DIR)/memory.h | $(BUILD_DIR)
	g++ -I/home/kraugher/Documents/officina/ELFIO -Wall -Wextra -std=c++17 -DLOG_LEVEL=$(LOG_LEVEL) -c $(SRC_DIR)/memory.cpp -o $(BUILD_DIR)/memory.o

$(BUILD_DIR)/main.o: $(SRC_DIR)/main.cpp $(SRC_DIR)/cpu.h $(SRC_DIR)/memory.h | $(BUILD_DIR)
	g++ -I/home/kraugher/Documents/officina/ELFIO -Wall -Wextra -std=c++17 -DLOG_LEVEL=$(LOG_LEVEL) -c $(SRC_DIR)/main.cpp -o $(BUILD_DIR)/main.o

# Directory necessarie
$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

$(BIN_DIR):
	mkdir -p $(BIN_DIR)

# Pulizia dei file generati
clean:
	rm -rf $(BUILD_DIR) $(BIN_DIR) $(TEST_DIR)/*.s $(TEST_DIR)/*.bin $(TEST_DIR)/*.dis $(TEST_DIR)/*.map
