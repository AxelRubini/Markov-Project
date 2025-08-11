# ---------------------------  Compiler & Flags -------------------------
CC      = gcc
CFLAGS  = -Wall -Wextra -std=c99 -O0 -g3

# ---------------------------  Directories ------------------------------
SRC_DIR   = ../src
INC_DIR   = ../include

# ---------------------------  Project Modules --------------------------
COMMON_CANDIDATES = $(wildcard $(SRC_DIR)/*.c) \
									 $(wildcard $(INC_DIR)/*.c)

COMMON_SRC = $(wildcard $(COMMON_CANDIDATES))
#----------------------------	Exclude main.c from common sources -------------------
COMMON_SRC := $(filter-out \
  $(SRC_DIR)/main.c \
,  $(COMMON_SRC))

# ---------------------------  Tests Auto-Discovery ---------------------
TEST_SRCS = $(wildcard test_*.c)
TESTS     = $(patsubst %.c,%,$(TEST_SRCS))

# ---------------------------  Default ---------------------------------
.PHONY: all run run-one clean list
all: $(TESTS)

# ---------------------------  Link & Compile ---------------------------
%: %.c $(COMMON_SRC)
	@echo "Building $@..."
	@$(CC) $(CFLAGS) -I$(INC_DIR) $^ -o $@ -lm 

# ---------------------------  Run All ----------------------------------
run: all
	@set -e; \
	for t in $(TESTS); do \
		echo "Running $$t..."; \
		./$$t; \
	done
	@echo "All tests completed."

# ---------------------------  Run Single -------------------------------
# Usage: make run-one ONE=test_read_csv_line
run-one: %: $(ONE)
	@if [ -z "$(ONE)" ]; then \
		echo "Error: specify ONE=<test_name>"; \
		exit 1; \
	fi
	@$(MAKE) $(ONE)
	@echo "Running $(ONE)..."
	@./$(ONE)

# ---------------------------  Utility ----------------------------------
list:
	@echo "CFLAGS = $(CFLAGS)"
	@echo "INC_DIR = $(INC_DIR)"
	@echo "SRC_DIR = $(SRC_DIR)"
	@echo "COMMON_SRC = $(COMMON_SRC)"
	@echo "TESTS = $(TESTS)"

# ---------------------------  Clean ------------------------------------
clean:
	@rm -f $(TESTS)
	@echo "Cleaned test executables."
