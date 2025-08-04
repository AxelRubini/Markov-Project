#include "../include/next_word_t.h"
#include <stdio.h>
#include <stdlib.h>

next_word_t *create_next_word(void *next_word){
    next_word_t *new_next_word = malloc(sizeof(next_word_t));
    if (new_next_word == NULL) {
        fprintf(stderr, "Memory allocation failed for next_word_t\n");
        return NULL;
    }
    new_next_word->next_word = next_word;
    new_next_word->frequency = FREQ_INIT; // Initialize frequency to FREQ_INIT
    return new_next_word;
}

void update_next_word_frequency(next_word_t *next_word) {
    if (next_word == NULL) {
        fprintf(stderr, "next_word_t is NULL\n");
        return;
    }
    next_word->frequency++;
}

void free_next_word(next_word_t *next_word) {
    if (next_word == NULL) {
        fprintf(stderr, "next_word_t is NULL\n");
        return;
    }
    free(next_word);
}
