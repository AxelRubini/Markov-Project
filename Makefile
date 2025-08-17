# ---------------------------  Programs Names ---------------------------
TARGET   = Markov_First

# ---------------------------  Compiler & Flags -------------------------
CC       = gcc
CSTD     = -std=gnu99
WARN     = -Wall -Wextra -Wpedantic
OPT_DBG  = -O0 -g3 -fno-omit-frame-pointer
OPT_REL  = -O2
CDEPS    = -MMD -MP

# Include dirs (aderenti alla tua tree)
CINCS    = -I$(INC_DIR)/csv -I$(INC_DIR)/io -I$(INC_DIR)/ds -I$(INC_DIR)/core \
           -I$(INC_LIB)/argtable3 -I$(INC_DIR)/app -I$(INC_DIR)/modes/sp \
           -I$(INC_DIR)/modes/mp -I$(INC_DIR)/modes

CFLAGS   = $(WARN) $(CSTD) $(OPT_DBG) $(CINCS) $(CDEPS)
LDFLAGS  =
LDLIBS   = -lm $(INC_LIB)/argtable3/libargtable3.a

# ---------------------------  Directories ------------------------------
SRC_DIR   = src
INC_DIR   = include
INC_LIB   = lib
BUILD_DIR = build
TEST_DIR  = test

#-----------------------------	Test Profile -------------------
TEST_PROFILE ?= debug 

# ---------------------------  Sources & Objects ------------------------
# Tutti i .c sotto src/ (ricorsivo), escluso eventuale file legacy
SRC       = $(shell find $(SRC_DIR) -name '*.c' ! -path "$(SRC_DIR)/ds/ht_data_manipulation.c")
OBJ       = $(patsubst $(SRC_DIR)/%.c,$(BUILD_DIR)/%.o,$(SRC))

# ---------------------------  Default ----------------------------------
.PHONY: all test debug release run list tests asan-tests valgrind test-one tests-clean clean distclean
all: debug

debug: CFLAGS := $(WARN) $(CSTD) $(OPT_DBG) $(CINCS) $(CDEPS)
debug: $(TARGET)

release: CFLAGS := $(WARN) $(CSTD) $(OPT_REL) $(CINCS) $(CDEPS)
release: $(TARGET)

# ---------------------------  Link -------------------------------------
$(TARGET): $(OBJ) | $(BUILD_DIR)
	$(CC) $^ $(LDFLAGS) $(LDLIBS) -o $@

# ---------------------------  Compile ----------------------------------
$(BUILD_DIR)/%.o: $(SRC_DIR)/%.c | $(BUILD_DIR)
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c $< -o $@

$(BUILD_DIR):
	@mkdir -p $@

# ---------------------------  Run --------------------------------------
run: $(TARGET)
	@MALLOC_CHECK_=3 MALLOC_PERTURB_=204 ./$(TARGET)

# ---------------------------  Tests (delegati) -------------------------
test: 
	@$(MAKE) -C $(TEST_DIR) test $(TEST_PROFILE)
	@echo "Starting tests in $(TEST_DIR)..."
	@for t in $(find bin/test -type f -executable); do \
		echo "Running $$t..."; \
		MALLOC_CHECK_=3 MALLOC_PERTURB_=204 ./$$t; \
	done
tests:
	@$(MAKE) -C $(TEST_DIR) tests

asan-tests:
	@$(MAKE) -C $(TEST_DIR) asan

# Esempio: make valgrind TEST=../bin/test/test_main_mp
valgrind:
	@$(MAKE) -C $(TEST_DIR) valgrind TEST=$(TEST)

# Esempio: make test-one ONE=test_main_mp
test-one:
	@$(MAKE) -C $(TEST_DIR) run-one ONE=$(ONE)

tests-clean:
	@$(MAKE) -C $(TEST_DIR) distclean

# ---------------------------  Utils ------------------------------------
list:
	@echo "SRC        = $(SRC)"
	@echo "OBJ        = $(OBJ)"
	@echo "CFLAGS     = $(CFLAGS)"

# ---------------------------  Clean ------------------------------------
clean:
	@rm -rf $(BUILD_DIR) $(TARGET)
	@echo "Cleaned build directory and executable."

distclean: clean
	@$(MAKE) -C $(TEST_DIR) distclean

# ---------------------------  Deps --------------------------------------
-include $(OBJ:.o=.d)
