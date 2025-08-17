# ---------------------------  Programs Names ---------------------------
TARGET = Markov_First

# ---------------------------  Compiler & Flags -------------------------
CC      = gcc
CFLAGS  = -Wall -Wextra -std=gnu99 -O0 -g3 \
          -I$(INC_DIR)/csv -I$(INC_DIR)/io -I$(INC_DIR)/ds -I$(INC_DIR)/core \
					-I$(INC_LIB)/argtable3 -I$(INC_DIR)/app -I$(INC_DIR)/modes/sp \
					-I$(INC_DIR)/modes/mp -I$(INC_DIR)/modes
LDLIBS = -lm $(INC_LIB)/argtable3/libargtable3.a

# ---------------------------  Directories ------------------------------
SRC_DIR   = src
INC_DIR   = include
INC_LIB		= lib
BUILD_DIR = build
TEST_DIR  = test            # test code lives here

# ---------------------------  Sources & Objects ------------------------
# Discover all source files recursively, excluding legacy ds implementation
SRC      = $(shell find $(SRC_DIR) -name '*.c' ! -path "$(SRC_DIR)/ds/ht_data_manipulation.c")
OBJ      = $(patsubst $(SRC_DIR)/%.c,$(BUILD_DIR)/%.o,$(SRC))

# ---------------------------  Libraries --------------------------------

# ---------------------------  Default ---------------------------------
.PHONY: all main
all: $(TARGET)

main: $(TARGET)

# ---------------------------  Link -------------------------------------
$(TARGET): $(BUILD_DIR) $(OBJ)
	@echo "Linking $@..."
	@$(CC) $(OBJ) $(LDFLAGS) $(LDLIBS) -o $@
	@echo "Build completed: $@"

# ---------------------------  Compile ----------------------------------
$(BUILD_DIR)/%.o: $(SRC_DIR)/%.c | $(BUILD_DIR)
	@mkdir -p $(dir $@)
	@echo "Compiling $<..."
	@$(CC) $(CFLAGS) -I$(INC_DIR) -c $< -o $@

# ---------------------------  Build Dir --------------------------------
$(BUILD_DIR):
	@mkdir -p $(BUILD_DIR)

# ---------------------------  Tests (/test) -------------------------
.PHONY: test test-one tests-clean
test:
	@$(MAKE) -C $(TEST_DIR) test

# Run a single test: make test-one NOME=<test_file.c>
test-one:
	@$(MAKE) -C $(TEST_DIR) one NOME=$(NOME)

tests-clean:
	@$(MAKE) -C $(TEST_DIR) distclean

# ---------------------------  Clean ------------------------------------
.PHONY: clean distclean
clean:
	@rm -rf $(BUILD_DIR) $(TARGET)
	@echo "Cleaned build directory and executable."

distclean: clean
	@$(MAKE) -C $(TEST_DIR) distclean
