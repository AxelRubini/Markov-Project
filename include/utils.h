#ifndef UTILS_H
#define UTILS_H

#include <math.h>

int is_prime(int n);
int next_prime(int n);
static unsigned int hash_function(int *key, int table_size);
void *dmalloc(size_t size);

#endif 