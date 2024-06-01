CC=gcc
CFLAGS=-Wall -pthread
BIN_DIR=./bin
SRC_DIR=./src

all: $(BIN_DIR) hair_salon1 hair_salon2

$(BIN_DIR):
	mkdir -p $(BIN_DIR)

hair_salon1: $(SRC_DIR)/hair_salon1.c
	$(CC) $(CFLAGS) $(SRC_DIR)/hair_salon1.c -o $(BIN_DIR)/hair_salon1

hair_salon2: $(SRC_DIR)/hair_salon2.c
	$(CC) $(CFLAGS) $(SRC_DIR)/hair_salon2.c -o $(BIN_DIR)/hair_salon2

clean:
	rm -f $(BIN_DIR)/hair_salon1
	rm -f $(BIN_DIR)/hair_salon2
	rmdir $(BIN_DIR)

.PHONY: all clean
