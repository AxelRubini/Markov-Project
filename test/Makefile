# Compiler and flags
CC ?= gcc
STD ?= -std=gnu99
WARN ?= -Wall -Wextra -Wpedantic
DEBUG ?= -O0 -g3 -fno-omit-frame-pointer

ROOT ?= ..
BUILD_TEST ?= $(ROOT)/build/test
BUILD_TESTCORE ?= $(ROOT)/build/test_core
BIN_TEST ?= $(ROOT)/bin/test

INCLUDES ?= -I$(ROOT)/include -I$(ROOT)/include/csv -I$(ROOT)/include/io -I$(ROOT)/include/ds -I$(ROOT)/include/core -I$(ROOT)/include/app -I$(ROOT)/include/modes/sp -I$(ROOT)/lib/argtable3

CFLAGS ?= $(STD) $(WARN) $(DEBUG) $(INCLUDES)

COMMON_SRCS := $(shell find $(ROOT)/src -name '*.c' ! -name 'main.c')
COMMON_OBJS := $(patsubst $(ROOT)/src/%.c,$(BUILD_TESTCORE)/%.o,$(COMMON_SRCS))
COMMON_DEPS := $(COMMON_OBJS:.o=.d)

TEST_SRCS := $(wildcard *_test.c test_*.c)
TEST_OBJS := $(patsubst %.c,$(BUILD_TEST)/%.o,$(TEST_SRCS))
TEST_DEPS := $(TEST_OBJS:.o=.d)
TEST_BINS := $(patsubst %.c,$(BIN_TEST)/%,$(TEST_SRCS))

.PHONY: all test run one clean distclean test-vars

all: $(TEST_BINS)

test: all
	@set -e; for t in $(TEST_BINS); do echo "Running $(notdir $$t)..."; $$t; done

run:
	@set -e; for t in $(TEST_BINS); do echo "Running $(notdir $$t)..."; $$t; done

one:
	@if [ -z "$(NOME)" ]; then echo "Error: specify NOME=<test_file.c>"; exit 1; fi; $(MAKE) $(BIN_TEST)/$(basename $(NOME)); echo "Running $(basename $(NOME))..."; $(BIN_TEST)/$(basename $(NOME))

%_test: $(BIN_TEST)/%_test
	@echo "Running $<..."; $<

$(BIN_TEST)/%: $(BUILD_TEST)/%.o $(COMMON_OBJS) | $(BIN_TEST)
	$(CC) $(CFLAGS) $^ -o $@ -lm $(ROOT)/lib/argtable3/libargtable3.a

$(BUILD_TEST)/%.o: %.c | $(BUILD_TEST)
	$(CC) $(CFLAGS) -c $< -o $@ $(ROOT)/lib/argtable3/libargtable3.a 

$(BUILD_TESTCORE)/%.o: $(ROOT)/src/%.c | $(BUILD_TESTCORE)
	$(CC) $(CFLAGS) -c $< -o $@ $(ROOT)/lib/argtable3/libargtable3.a 

$(BUILD_TEST) $(BUILD_TESTCORE) $(BIN_TEST):
	@mkdir -p $@

test-vars:
	@echo "ROOT=$(ROOT)"
	@echo "BUILD_TEST=$(BUILD_TEST)"
	@echo "BUILD_TESTCORE=$(BUILD_TESTCORE)"
	@echo "BIN_TEST=$(BIN_TEST)"
	@echo "COMMON_SRCS=$(COMMON_SRCS)"
	@echo "COMMON_OBJS=$(COMMON_OBJS)"
	@echo "TEST_SRCS=$(TEST_SRCS)"
	@echo "TEST_OBJS=$(TEST_OBJS)"
	@echo "TEST_BINS=$(TEST_BINS)"

clean:
	@rm -rf $(BUILD_TEST) $(BUILD_TESTCORE)
	@echo "Cleaned test build directories."

distclean: clean
	@rm -rf $(BIN_TEST)
	@echo "Removed test binaries."

-include $(COMMON_DEPS) $(TEST_DEPS)

distclean: clean
	@rm -rf $(BIN_TEST)
	@echo "Distclean test: eseguibili rimossi."


