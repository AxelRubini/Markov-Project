#include "../include/utils.h"


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