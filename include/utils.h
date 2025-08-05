#ifndef UTILS_H
#define UTILS_H

#include <math.h>
#include <stdlib.h>

unsigned int is_prime(int n);
unsigned int next_prime(int n);
unsigned int hash_function(int *key, int table_size);
void *dmalloc(size_t size);

#endif
