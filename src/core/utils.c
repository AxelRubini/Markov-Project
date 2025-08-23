#include "utils.h"

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <unistd.h>

uint32_t is_prime(int n)
{
	if (n <= 1)
		return 0;
	if (n <= 3)
		return 1;
	int max_to_divide = (int)sqrt(n);
	for (int i = 2; i <= max_to_divide; i++) {
		if (n % i == 0) {
			return 0; // not prime
		}
	}
	return 1; // prime
}

uint32_t next_prime(int n)
{
	if (n <= 1)
		return 2;
	if (n % 2 == 0)
		n++; // make it odd
	while (!is_prime(n)) {
		n += 2; // check only odd numbers
	}
	return n;
}

uint32_t hash_function(const int *key, int table_size)
{
	uint32_t hash = 5381;
	int c;

	while ((c = *key++)) {
		hash = ((hash << 5) + hash) + c; // hash * 33 + c
	}

	return hash % (uint32_t)table_size;
}

void *dmalloc(size_t size)
{
	void *ptr = malloc(size);
	if (ptr == NULL) {
		fprintf(stderr, "Memory allocation failed\n");
		exit(EXIT_FAILURE);
	}
	memset(ptr, 0, size); // Initialize allocated memory to zero
	return ptr;
}

double next_word_probability(const int *word, const int *next_word)
{
	double probability = 0.0;
	probability =
		(double)(*next_word) / (double)(*word); // Example calculation
	return probability;
}

int safe_close(int *fd)
{
	if (fd == NULL || *fd < 0) {
		return 0;
	}
	if (close(*fd) < 0) {
		perror("safe_close");
		return -1;
	}
	*fd = -1;
	return 0;
}

int safe_close_pipes(int *fds, size_t count)
{
	if (fds == NULL || count == 0) {
		return 0; // Nothing to close
	}
	for (size_t i = 0; i < count; i++) {
		if (safe_close(&fds[i]) < 0) {
			return -1; // Error closing one of the pipes
		}
	}
	return 0; // All pipes closed successfully
}
