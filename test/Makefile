# =========================== Toolchain & Flags ===========================
CC       := gcc
STD      := -std=gnu99
WARN     := -Wall -Wextra -Wpedantic
OPT_DBG  := -O0 -g3 -fno-omit-frame-pointer
OPT_REL  := -O2
CDEPS    := -MMD -MP

# ----------------------------- Paths ------------------------------------
ROOT     := ..
SRC_DIR  := $(ROOT)/src
INC_DIR  := $(ROOT)/include
LIB_DIR  := $(ROOT)/lib

INCLUDES := -I$(INC_DIR)/csv -I$(INC_DIR)/io -I$(INC_DIR)/ds -I$(INC_DIR)/core \
            -I$(LIB_DIR)/argtable3 -I$(INC_DIR)/app -I$(INC_DIR)/modes/sp \
            -I$(INC_DIR)/modes/mp -I$(INC_DIR)/modes

CFLAGS_BASE := $(STD) $(WARN) $(INCLUDES) $(CDEPS)
LDFLAGS     :=
LDLIBS      := -lm $(LIB_DIR)/argtable3/libargtable3.a

# ----------------------------- Layout -----------------------------------
BUILD_TEST     := $(ROOT)/build/test
BUILD_TESTCORE := $(ROOT)/build/test_core
BIN_TEST       := $(ROOT)/bin/test

# ----------------------------- Sources ----------------------------------
COMMON_SRCS := $(shell find $(SRC_DIR) -name '*.c' ! -name 'main.c')
COMMON_OBJS := $(patsubst $(SRC_DIR)/%.c,$(BUILD_TESTCORE)/%.o,$(COMMON_SRCS))

TEST_SRCS := $(wildcard *.c)
TEST_OBJS := $(patsubst %.c,$(BUILD_TEST)/%.o,$(TEST_SRCS))
TEST_BINS := $(patsubst %.c,$(BIN_TEST)/%,$(TEST_SRCS))

# ---------------------------- Profiles ----------------------------------
CFLAGS_DBG := $(CFLAGS_BASE) $(OPT_DBG)
CFLAGS_REL := $(CFLAGS_BASE) $(OPT_REL)
CFLAGS_SAN := $(CFLAGS_BASE) $(OPT_DBG) -fsanitize=address,undefined
LDFLAGS_SAN:= -fsanitize=address,undefined


# ----------------------------- Targets ----------------------------------
.PHONY: all test debug release asan tests run-one valgrind clean distclean list

all: debug
 

debug: CFLAGS := $(CFLAGS_DBG)
debug: tests

release: CFLAGS := $(CFLAGS_REL)
release: tests

asan: CFLAGS := $(CFLAGS_SAN)
asan: LDFLAGS += $(LDFLAGS_SAN)
asan: tests

tests: $(BIN_TEST) $(TEST_BINS)

# Singolo test: make run-one ONE=test_main_mp
run-one: $(BIN_TEST)/$(ONE)
	@MALLOC_CHECK_=3 MALLOC_PERTURB_=204 ./$(BIN_TEST)/$(ONE)

$(BIN_TEST)/%: $(BUILD_TEST)/%.o $(COMMON_OBJS) | $(BIN_TEST)
	$(CC) $^ $(LDFLAGS) $(LDLIBS) -o $@

$(BUILD_TEST)/%.o: %.c
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c $< -o $@

$(BUILD_TESTCORE)/%.o: $(SRC_DIR)/%.c
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c $< -o $@

$(BIN_TEST):
	@mkdir -p $@

VALGRIND ?= valgrind --leak-check=full --show-leak-kinds=all --errors-for-leak-kinds=definite \
            --track-origins=yes --error-exitcode=99
TEST ?= $(firstword $(TEST_BINS))

valgrind: debug
	@test -n "$(TEST)" || (echo "Nessun test trovato"; exit 2)
	@echo ">>> VALGRIND su $(TEST)"
	@MALLOC_CHECK_=3 MALLOC_PERTURB_=204 $(VALGRIND) --log-file=valgrind.out ./$(TEST)

list:
	@echo "COMMON_SRCS = $(COMMON_SRCS)"
	@echo "TEST_SRCS   = $(TEST_SRCS)"
	@echo "TEST_BINS   = $(TEST_BINS)"
	@echo "CFLAGS      = $(CFLAGS)"

clean:
	@rm -rf $(BUILD_TEST) $(BUILD_TESTCORE)
	@echo "Clean test: oggetti rimossi."

distclean: clean
	@rm -rf $(BIN_TEST)
	@echo "Distclean test: eseguibili rimossi."

# Includo i file .d generati da -MMD -MP (ricompilo se cambiano gli .h)
-include $(COMMON_OBJS:.o=.d)
-include $(TEST_OBJS:.o=.d)
