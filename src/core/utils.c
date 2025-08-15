#include "utils.h"

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

uint32_t is_prime(int n) {

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

uint32_t next_prime(int n) {
  if (n <= 1)
    return 2;
  if (n % 2 == 0)
    n++; // make it odd
  while (!is_prime(n)) {
    n += 2; // check only odd numbers
  }
  return n;
}

uint32_t hash_function(const int *key, int table_size) {

  uint32_t hash = 5381;
  int c;

  while ((c = *key++)) {
    hash = ((hash << 5) + hash) + c; // hash * 33 + c
  }

  return hash % (uint32_t)table_size;
}

void *dmalloc(size_t size) {
  void *ptr = malloc(size);
  if (ptr == NULL) {
    fprintf(stderr, "Memory allocation failed\n");
    exit(EXIT_FAILURE);
  }
  memset(ptr, 0, size); // Initialize allocated memory to zero
  return ptr;
}
#define malloc(x) dont_use_malloc_usedmalloc // Use dmalloc instead

double next_word_probability(const int *word, const int *next_word) {
  double probability = 0.0;
  probability = (double)(*next_word) / (double)(*word); // Example calculation
  return probability;
}
