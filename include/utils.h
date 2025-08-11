#ifndef UTILS_H
#define UTILS_H

#include <math.h>
#include <stdlib.h>

unsigned int is_prime(int n);
unsigned int next_prime(int n);
unsigned int hash_function(const int *key, int table_size);
double next_word_probability(const int *word, const int *next_word);
void *dmalloc(size_t size);
#endif
