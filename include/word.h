#ifndef WORD_H
#define WORD_H

#include "ht_item.h"
#include "linked_list.h"
#include <stdio.h>
#include <stdlib.h>

/*This structure represents a word and its occurrence count*/

#define MAX_WORD_LENGTH                                                        \
  30 // Maximum length of a word in code points with null terminator
typedef struct {
  int *word; // pointer to the first character(code point) of the word (integer
             // array)
  int occurrences; // number of occurrences of the word
} word_t;

word_t *create_word(int *word);
void update_word_occurrences(word_t *word);
int get_word_occurrences(const word_t *word);
int wordcmp(const word_t *word1, const word_t *word2);
int word_str_cmp(const int *word, const int *str);
void update_ht_item_value(const void *item, const void *new_value);
ht_item *word_ht_item_create(word_t *key, word_t *value);
unsigned int word_hash(const void *key, int size);
void print_utf8_word(const word_t *word, int fd);
void word_print(const word_t *word, int fd, int *between_char);
void free_word(word_t *word);
void ht_item_free_word(ht_item *item);
static void word_followers_free_list(linked_list_t *list);
int word_hashtable_keycmp(const void *key1, const void *key2);
static word_t *word_deep_copy(const word_t *original);

#endif
