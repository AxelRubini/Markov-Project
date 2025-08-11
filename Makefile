# ---------------------------  Programs Names ---------------------------
TARGET = Markov_First

# ---------------------------  Compiler & Flags -------------------------
CC      = gcc
CFLAGS  = -Wall -Wextra -std=c99 -O0 -g3
LDFLAGS = -lm 

# ---------------------------  Directories ------------------------------
SRC_DIR   = src
INC_DIR   = include
BUILD_DIR = build
TEST_DIR  = test            # test code lives here

# ---------------------------  Sources & Objects ------------------------
# Wildcard all .c files in the source directory
SRC      = $(wildcard $(SRC_DIR)/*.c)
OBJ      = $(patsubst $(SRC_DIR)/%.c,$(BUILD_DIR)/%.o,$(SRC))

# ---------------------------  Default ---------------------------------
.PHONY: all
all: $(TARGET)

# ---------------------------  Link -------------------------------------
$(TARGET): $(BUILD_DIR) $(OBJ)
	@echo "Linking $@..."
	@$(CC) $(OBJ) $(LDFLAGS) -o $@
	@echo "Build completed: $@"

# ---------------------------  Compile ----------------------------------
$(BUILD_DIR)/%.o: $(SRC_DIR)/%.c | $(BUILD_DIR)
	@echo "Compiling $<..."
	@$(CC) $(CFLAGS) -I$(INC_DIR) -c $< -o $@

# ---------------------------  Build Dir --------------------------------
$(BUILD_DIR):
	@mkdir -p $(BUILD_DIR)

# ---------------------------  Tests (/test) -------------------------
.PHONY: test test-one tests-clean
test:
	@$(MAKE) -C $(TEST_DIR) run

# Run a single test: make test-one ONE=<test_name>
test-one:
	@$(MAKE) -C $(TEST_DIR) run-one ONE=$(ONE)

tests-clean:
	@$(MAKE) -C $(TEST_DIR) clean

# ---------------------------  Clean ------------------------------------
.PHONY: clean distclean
clean:
	@rm -rf $(BUILD_DIR) $(TARGET)
	@echo "Cleaned build directory and executable."

distclean: clean
	@$(MAKE) -C $(TEST_DIR) clean
