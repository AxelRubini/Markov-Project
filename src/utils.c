#include "../include/utils.h"

#include <stdio.h>
#include <stdlib.h>


static unsigned int is_prime(int n) {

    if (n <= 1) return 0;
    if (n <= 3) return 1;
    int max_to_divide = (int)sqrt(n);
    for (int i = 2; i <= max_to_divide; i++) {
        if (n % i == 0) {
            return 0; // not prime
        }
    }
    return 1; // prime
    
}

static int next_prime(int n) {
    if (n <= 1) return 2;
    if (n % 2 == 0) n++; // make it odd
    while (!is_prime(n)) {
        n += 2; // check only odd numbers
    }
    return n;
}

static unsigned int hash_function(int *key, int table_size){

    unsigned int hash = 5381;
    int c;

    while ((c = *key++)) {
        hash = ((hash << 5) + hash) + c; // hash * 33 + c
    }

    return hash % table_size;
}

void *dmalloc(size_t size) {
    void *ptr = malloc(size);
    if (ptr == NULL) {
        fprintf(stderr, "Memory allocation failed\n");
        exit(EXIT_FAILURE);
    }
    return ptr;
}
