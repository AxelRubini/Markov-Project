
# ---------------------------  Programs Names ---------------------------
TARGET = Markov_First

TEST_UTF8       = $(BUILD_DIR)/utf8            # Test for UTF-8 handling
TEST_DATA_STRUCT = $(BUILD_DIR)/data_struct_test # Test for data structures

# ---------------------------  Compiler & flags -------------------------
CC      = gcc
CFLAGS  = -Wall -Wextra -std=c99 -O0 -g3

# ---------------------------  Directories ------------------------------
SRC_DIR   = src
TEST_DIR  = test
BUILD_DIR = build

# ---------------------------  Sources & objects ------------------------
# Program principal
SRC = $(SRC_DIR)/main.c \
      $(SRC_DIR)/ht.c $(SRC_DIR)/linked_list.c $(SRC_DIR)/utf8_tools.c \
      $(SRC_DIR)/utils.c $(SRC_DIR)/word.c $(SRC_DIR)/ht_item.c
OBJ = $(patsubst %.c,$(BUILD_DIR)/%.o,$(SRC))

# Test UTF-8
TEST_UTF8_SRC = $(TEST_DIR)/test_utf8.c $(SRC_DIR)/utf8_tools.c
TEST_UTF8_OBJ = $(patsubst %.c,$(BUILD_DIR)/%.o,$(TEST_UTF8_SRC))

# Test strutture dati
TEST_DATA_STRUCT_SRC = $(TEST_DIR)/test_ds.c \
                       $(SRC_DIR)/linked_list.c \
                       $(SRC_DIR)/ht_item.c $(SRC_DIR)/utils.c \
                       $(SRC_DIR)/word.c $(SRC_DIR)/utf8_tools.c \
											 $(SRC_DIR)/hash_table.c
TEST_DATA_STRUCT_OBJ = $(patsubst %.c,$(BUILD_DIR)/%.o,$(TEST_DATA_STRUCT_SRC))

# ---------------------------  Phony targets ----------------------------
.PHONY: all clean test_utf8 test test_data_struct

# ---------------------------  Build rules ------------------------------
#  math.h need to be linked with -lm
all: $(TARGET)

$(TARGET): $(OBJ)
	$(CC) $(CFLAGS) -o $@ $^ -lm

# --- Test UTF-8 executable
$(TEST_UTF8): $(TEST_UTF8_OBJ)
	$(CC) $(CFLAGS) -o $@ $^ -lm

# --- Test strutture dati executable
$(TEST_DATA_STRUCT): $(TEST_DATA_STRUCT_OBJ)
	$(CC) $(CFLAGS) -o $@ $^ -lm

# --- Pattern rule for objects (TAB obbligatorio davanti al CC)
$(BUILD_DIR)/%.o: %.c
	@mkdir -p $(@D)
	$(CC) $(CFLAGS) -c $< -o $@

# ---------------------------  Test targets -----------------------------
test_utf8: $(TEST_UTF8)
	@echo "Running UTF-8 handling test..."
	@./$(TEST_UTF8)
	@echo "Test completed."

test_data_struct: $(TEST_DATA_STRUCT)
	@echo "Running data structures test..."
	@./$(TEST_DATA_STRUCT)
	@echo "Data structures test completed."

test: test_utf8 test_data_struct
	@echo "All tests completed."

# ---------------------------  Clean ------------------------------------
clean:
	@rm -rf $(BUILD_DIR) $(TARGET) $(TEST_UTF8) $(TEST_DATA_STRUCT)

