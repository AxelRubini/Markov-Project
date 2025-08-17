# ---------------------------  Program Name ---------------------------
TARGET = Markov_First

# ---------------------------  Compiler & Flags -----------------------
CC      = gcc
CSTD    = -std=gnu99
WARN    = -Wall -Wextra -Wpedantic
OPT_DBG = -O0 -g3 -fno-omit-frame-pointer
OPT_REL = -O2

# Include directories
CINCS = -I$(INC_DIR)/csv -I$(INC_DIR)/io -I$(INC_DIR)/ds -I$(INC_DIR)/core         -I$(INC_LIB)/argtable3 -I$(INC_DIR)/app -I$(INC_DIR)/modes/sp         -I$(INC_DIR)/modes/mp -I$(INC_DIR)/modes

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
.PHONY: all main run test tests test-one tests-clean debug release clean distclean
all: $(TARGET)

main: $(TARGET)

debug: CFLAGS := $(WARN) $(CSTD) $(OPT_DBG) $(CINCS)
debug: clean all

release: CFLAGS := $(WARN) $(CSTD) $(OPT_REL) $(CINCS)
release: clean all

run: $(TARGET)
	./$(TARGET)

test tests:
	$(MAKE) -C $(TEST_DIR) test

test-one:
	$(MAKE) -C $(TEST_DIR) one NOME=$(NOME)

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

distclean: clean
	$(MAKE) -C $(TEST_DIR) distclean
