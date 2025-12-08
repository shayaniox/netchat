CC        := gcc
SRC_DIR   := src
INC_DIR   := include
BUILD_DIR := build
BIN_FILE  := $(BUILD_DIR)/nchat

CFLAGS  := -std=c11 -Wall -Wextra -g -I $(INC_DIR)
LDFLAGS :=
LDLIBS  :=

SRC_FILES := $(wildcard $(SRC_DIR)/*.c)
OBJ_FILES := $(patsubst $(SRC_DIR)/%.c,$(BUILD_DIR)/%.o,$(SRC_FILES))

$(BIN_FILE): $(OBJ_FILES)
	$(CC) $(CFLAGS) $^ -o $@

$(BUILD_DIR)/%.o: $(SRC_DIR)/%.c | $(BUILD_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

$(BUILD_DIR):
	@mkdir -p $(BUILD_DIR)

install: $(BUILD_DIR)/$(BIN_FILE)
	install -d

clean:
	rm -rf $(BUILD_DIR)
