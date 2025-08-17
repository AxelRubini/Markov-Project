# ---------------------------  Program Name ---------------------------
TARGET = Markov_First

# ---------------------------  Compiler & Flags -----------------------
CC      = gcc
CSTD    = -std=gnu99
WARN    = -Wall -Wextra -Wpedantic
OPT_DBG = -O0 -g3 -fno-omit-frame-pointer
OPT_REL = -O2

# Include directories
CINCS = -I$(INC_DIR)/csv -I$(INC_DIR)/io -I$(INC_DIR)/ds -I$(INC_DIR)/core \
        -I$(INC_LIB)/argtable3 -I$(INC_DIR)/app -I$(INC_DIR)/modes/sp \
        -I$(INC_DIR)/modes/mp -I$(INC_DIR)/modes

CFLAGS = $(WARN) $(CSTD) $(OPT_DBG) $(CINCS)
LDLIBS = -lm $(INC_LIB)/argtable3/libargtable3.a

# ---------------------------  Directories ----------------------------
SRC_DIR   = src
INC_DIR   = include
INC_LIB   = lib
BUILD_DIR = build
TEST_DIR  = test

# ---------------------------  Sources --------------------------------
SRC = $(shell find $(SRC_DIR) -name '*.c' ! -path "$(SRC_DIR)/ds/ht_data_manipulation.c")
OBJ = $(patsubst $(SRC_DIR)/%.c,$(BUILD_DIR)/%.o,$(SRC))

# ---------------------------  Targets --------------------------------
TEST_TARGETS := test tests run one valgrind debug release asan ubsan

.PHONY: all main run-app tests-clean $(TEST_TARGETS) test-proxy clean distclean

all: $(TARGET)

main: $(TARGET)

run-app: $(TARGET)
	./$(TARGET)



$(TEST_TARGETS): test-proxy
	@:

%_test: test-proxy
	@:

test_%: test-proxy
	@:

test-proxy:
	$(MAKE) -C $(TEST_DIR) $(MAKECMDGOALS) $(if $(NOME),NOME=$(NOME),)

tests-clean:
	$(MAKE) -C $(TEST_DIR) distclean

# ---------------------------  Build ----------------------------------
$(TARGET): $(OBJ) | $(BUILD_DIR)
	$(CC) $^ $(LDLIBS) -o $@

$(BUILD_DIR)/%.o: $(SRC_DIR)/%.c | $(BUILD_DIR)
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c $< -o $@

$(BUILD_DIR):
	@mkdir -p $@

# ---------------------------  Clean ----------------------------------
clean:
	@rm -rf $(BUILD_DIR) $(TARGET)
	@echo "Cleaned build directory and executable."

distclean: clean tests-clean
