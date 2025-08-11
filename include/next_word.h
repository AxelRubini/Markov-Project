#ifndef NEXT_WORD_H
#define NEXT_WORD_H

#include "ht_item.h"
#include "linked_list.h"
#include "utf8_tools.h"

/*
 * The next_word_t structure represents a word as string of UTF-8 codepoints
 * and its probability as following word respectrfully to the key of the relative ht_item.
 * In this header file, we define the structure and functions to create,
 * manipulate, and print next_word_t objects and define the specialization to create
 * a hash table item (ht_item) that uses next_word_t as value.
 */

typedef struct next_word_t next_word_t;

struct next_word_t {
  int *word;
  double probability;
};

next_word_t *create_next_word(const int *word, double problability);
ht_item *next_word_ht_item_create(const int *word, linked_list_t *next_word);
void free_next_word(next_word_t *next_word);
void free_next_word_ht_item(ht_item *item);
void update_ht_item_next_word(const void *item, const void *next_word_to_add);
void print_next_word(const next_word_t *next_word, int fd);
void print_ht_item_next_word(const ht_item *item, int fd);
int keycmp_next_word(const void *key1, const void *key2);
unsigned int next_word_hash(const void *key, int table_size);

#endif // NEXT_WORD_H
