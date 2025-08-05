#ifndef WORD_H
#define WORD_H

#include <stdio.h>
#include <stdlib.h>
/*This structure represents a word and its occurrence count*/
typedef struct {
    int *word; // pointer to the first character(code point) of the word (integer array)
    int occurrences; // number of occurrences of the word
} word_t;

word_t *create_word(int *word);
void update_word_occurrences(word_t *word);
int get_word_occurrences(const word_t *word);

int wordcmp_no_caseSensitive(const word_t *word1, const word_t *word2);
void free_word(word_t *word);

#endif