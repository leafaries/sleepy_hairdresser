CC=gcc
CFLAGS=-Wall -pthread
BIN_DIR=./bin
SRC_DIR=./src

all: $(BIN_DIR) hair_salon1 hair_salon2

$(BIN_DIR):
	mkdir -p $(BIN_DIR)

hair_salon1: $(SRC_DIR)/hair_salon1.c $(SRC_DIR)/queue.h
	$(CC) $(CFLAGS) $(SRC_DIR)/queue.c $(SRC_DIR)/hair_salon1.c -o $(BIN_DIR)/hair_salon1

hair_salon2: $(SRC_DIR)/hair_salon2.c $(SRC_DIR)/queue.h
	$(CC) $(CFLAGS) $(SRC_DIR)/queue.c $(SRC_DIR)/hair_salon2.c -o $(BIN_DIR)/hair_salon2

clean:
	rm -f $(BIN_DIR)/hair_salon1
	rm -f $(BIN_DIR)/hair_salon2

run1:
	$(BIN_DIR)/hair_salon1

run1-info:
	$(BIN_DIR)/hair_salon1 -info

run2:
	$(BIN_DIR)/hair_salon2

run2-info:
	$(BIN_DIR)/hair_salon2 -info

.PHONY: all clean run1 run1-info run2 run2-info
