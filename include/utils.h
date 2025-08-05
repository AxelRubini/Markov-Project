#ifndef UTILS_H
#define UTILS_H

#include <math.h>

int is_prime(int n);
int next_prime(int n);
static unsigned int hash_function(int *key, int table_size){

    unsigned int hash = 5381;
    int c;

    while ((c = *key++)) {
        hash = ((hash << 5) + hash) + c; // hash * 33 + c
    }

    return hash % table_size;
}

#endif 