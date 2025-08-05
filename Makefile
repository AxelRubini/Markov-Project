# Programs Names
TARGET = Markov_First

TEST_UTF8 = $(BUILD_DIR)/utf8 # Test for UTF-8 handling


# Compiler and flags
CC = gcc
CFLAGS = -Wall -Wextra -std=c99 -g

# Directories paths
SRC_DIR = src
TEST_DIR = test
BUILD_DIR = build

# Files for main build
# patsubst is used to convert source files path to object files path using the build directory (pattern substitution with words)
SRC = $(SRC_DIR)/main.c $(SRC_DIR)/utf8_reader.c
OBJ = $(patsubst %.c,$(BUILD_DIR)/%.o,$(SRC))

# Files for test UTF-8 handling
TEST_UTF8_SRC = $(TEST_DIR)/test_utf8.c $(SRC_DIR)/utf8_reader.c
TEST_UTF8_OBJ = $(patsubst %.c,$(BUILD_DIR)/%.o,$(TEST_UTF8_SRC))

# Files for data structures testing
#
TEST_DATA_STRUCT_SRC = $(TEST_DIR)/test_data_struct.c $(SRC_DIR)/linked_list.c $(SRC_DIR)/next_word.c
TEST_DATA_STRUCT_OBJ = $(patsubst %.c,$(BUILD_DIR)/%.o,$(TEST_DATA_STRUCT_SRC))

.PHONY: all clean test_utf8 test

all: $(TARGET) 

# Build the main executable
$(TARGET): $(OBJ)
	$(CC) $(CFLAGS) -o $@ $^

# Build Test Executable for UTF-8 handling
$(TEST_UTF8) : $(TEST_UTF8_OBJ)
	$(CC) $(CFLAGS) -o $@ $^

# Generic rules 
$(BUILD_DIR)/%.o: %.c
	@mkdir -p $(@D)
	$(CC) $(CFLAGS) -c $< -o $@


# Automatic test for UTF-8 handling
test_utf8: $(TEST_UTF8)
	@echo "Running UTF-8 handling test..."
	@./$(TEST_UTF8)
	@echo "Test completed."

test: test_utf8
	@echo "Running all tests..."
	@./$(TEST_UTF8)
	@echo "All tests completed."

# Clean up build files
clean:
	rm -rf $(BUILD_DIR)/*.o $(TARGET) $(TEST_UTF8)



