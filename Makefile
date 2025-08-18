CC = gcc
CFLAGS = -std=gnu99 -Wall -Wextra -Iinclude -Iinclude/core -Iinclude/csv -Iinclude/ds -Iinclude/io -Ilib/argtable3 -Iinclude/app -Iinclude/modes/sp -Iinclude/modes/mp -Iinclude/modes
LIBS = lib/argtable3/libargtable3.a -lm

SRCS := $(shell find src -name '*.c' ! -path '*/.*')
OBJS := $(SRCS:.c=.o)
TARGET = markov

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) $^ $(LIBS) -o $@

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(OBJS) $(TARGET)

.PHONY: all clean
