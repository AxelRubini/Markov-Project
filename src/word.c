#include "../include/word.h"
#include <stdlib.h>
#include <stdio.h>


word_t *create_word(int *word){
    if (word == NULL){
        fprintf(stderr, "Word is NULL\n");
        return NULL;
    }

    word_t *new_word = dmalloc(sizeof(word_t));


    new_word->word = word;
    new_word->occurrences = 1;

    return new_word;
}

void update_word_occurrences(word_t *word) {
    if (word == NULL) {
        fprintf(stderr, "Word is NULL\n");
        return;
    }

    word->occurrences++;
}

int get_word_occurrences(const word_t *word) {
    if (word == NULL) {
        fprintf(stderr, "Word is NULL\n");
        return 0;
    }

    return word->occurrences;
}

int wordcmp_no_caseSensitive(const word_t *word1, const word_t *word2) {
    if (word1 == NULL || word2 == NULL) {
        fprintf(stderr, "One or both words are NULL\n");
        return 0;
    }

    int i = 0;
    while (word1->word[i] != '\0' && word2->word[i] != '\0') {
        int char1 = utf8_char_to_lower(word1->word[i]);
        int char2 = utf8_char_to_lower(word2->word[i]);
        if (char1 != char2) {
            return char1 - char2;
        }
        i++;
    }
    return word1->word[i] - word2->word[i];
}

void free_word(word_t *word) {
    if (word == NULL) {
        return;
    }

    free(word);
}