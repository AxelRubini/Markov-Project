# Esempi di utilizzo:
#   make test                # compila ed esegue tutti i test
#   make run                 # esegue senza ricompilare
#   make one NOME=foo_test.c # esegue un solo test
#   make foo_test            # shortcut per un singolo test
#   make asan test           # build con AddressSanitizer ed esecuzione
#   make valgrind            # esegue i test sotto Valgrind

CC      ?= gcc
STD     ?= -std=gnu99
WARN    ?= -Wall -Wextra -Wpedantic

ROOT       ?= ..
BUILD_TEST ?= $(ROOT)/build/test
BUILD_CORE ?= $(ROOT)/build/test_core
BIN_TEST   ?= $(ROOT)/bin/test

INCLUDES ?= -I$(ROOT)/include -I$(ROOT)/include/csv -I$(ROOT)/include/io \
            -I$(ROOT)/include/ds -I$(ROOT)/include/core -I$(ROOT)/include/app \
            -I$(ROOT)/include/modes/sp -I$(ROOT)/include/modes/mp \
            -I$(ROOT)/include/modes -I$(ROOT)/lib/argtable3

PROFILE ?= debug

ifneq ($(filter release,$(MAKECMDGOALS)),)
  PROFILE := release
endif
ifneq ($(filter asan,$(MAKECMDGOALS)),)
  PROFILE := asan
endif
ifneq ($(filter ubsan,$(MAKECMDGOALS)),)
  PROFILE := ubsan
endif
ifneq ($(filter debug,$(MAKECMDGOALS)),)
  PROFILE := debug
endif

DEBUG_FLAGS   = -O0 -g3 -fno-omit-frame-pointer
RELEASE_FLAGS = -O2
ASAN_FLAGS    = -fsanitize=address -fno-omit-frame-pointer
UBSAN_FLAGS   = -fsanitize=undefined -fno-omit-frame-pointer

ifeq ($(PROFILE),debug)
  OPT = $(DEBUG_FLAGS)
  SAN =
else ifeq ($(PROFILE),release)
  OPT = $(RELEASE_FLAGS)
  SAN =
else ifeq ($(PROFILE),asan)
  OPT = $(DEBUG_FLAGS) $(ASAN_FLAGS)
  SAN = $(ASAN_FLAGS)
else ifeq ($(PROFILE),ubsan)
  OPT = $(DEBUG_FLAGS) $(UBSAN_FLAGS)
  SAN = $(UBSAN_FLAGS)
else
  $(error Unknown PROFILE: $(PROFILE))
endif

CFLAGS  = $(STD) $(WARN) $(OPT) $(INCLUDES)
LDFLAGS = $(SAN)
LDLIBS  = -lm $(ROOT)/lib/argtable3/libargtable3.a

COMMON_SRCS := $(shell find $(ROOT)/src -type f -name '*.c' \
                  -not -name 'main.c' \
                  -not -name '.null-ls_*' \
                  -not -path '*/.*')
COMMON_OBJS := $(patsubst $(ROOT)/src/%.c,$(BUILD_CORE)/%.o,$(COMMON_SRCS))
COMMON_DEPS := $(COMMON_OBJS:.o=.d)

TEST_SRCS := $(wildcard *_test.c) $(wildcard test_*.c)
TEST_OBJS := $(patsubst %.c,$(BUILD_TEST)/%.o,$(TEST_SRCS))
TEST_DEPS := $(TEST_OBJS:.o=.d)
TEST_BINS := $(patsubst %.c,$(BIN_TEST)/%,$(TEST_SRCS))

VALGRIND_OPTS = --leak-check=full --track-origins=yes --error-exitcode=99

.PHONY: all test run one clean distclean debug release asan ubsan valgrind

all: $(TEST_BINS)

test: all
	@if [ -z "$(TEST_BINS)" ]; then echo "No tests found."; else \
	for t in $(TEST_BINS); do echo "Running $(notdir $$t)..."; $$t; done; fi

run:
	@if [ -z "$(TEST_BINS)" ]; then echo "No tests found."; else \
	for t in $(TEST_BINS); do echo "Running $(notdir $$t)..."; $$t; done; fi

one:
	@if [ -z "$(NOME)" ]; then \\
	echo "Error: specify NOME=<test_file.c>"; exit 1; \\
	fi; \\
	$(MAKE) $(BIN_TEST)/$(basename $(NOME)) PROFILE=$(PROFILE); \\
	echo "Running $(basename $(NOME))..."; \\
	$(BIN_TEST)/$(basename $(NOME))

%_test: $(BIN_TEST)/%_test
	@echo "Running $<..."; $<

$(BIN_TEST)/%: $(BUILD_TEST)/%.o $(COMMON_OBJS) | $(BIN_TEST)
	$(CC) $(CFLAGS) $(LDFLAGS) $^ -o $@ $(LDLIBS)

$(BUILD_TEST)/%.o: %.c | $(BUILD_TEST)
	$(CC) $(CFLAGS) -MMD -MP -c $< -o $@

$(BUILD_CORE)/%.o: $(ROOT)/src/%.c | $(BUILD_CORE)
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -MMD -MP -c $< -o $@

$(BUILD_TEST) $(BUILD_CORE) $(BIN_TEST):
	@mkdir -p $@

valgrind: all
	@if [ -z "$(TEST_BINS)" ]; then echo "No tests found."; else \
	for t in $(TEST_BINS); do \
	echo "Running $(notdir $$t) under Valgrind..."; \
	valgrind $(VALGRIND_OPTS) $$t; \
	done; fi

clean:
	@rm -rf $(BUILD_TEST) $(BUILD_CORE)
	@echo "Cleaned test build directories."

distclean: clean
	@rm -rf $(BIN_TEST)
	@echo "Removed test binaries."

-include $(COMMON_DEPS) $(TEST_DEPS)

debug release asan ubsan:
	@:

