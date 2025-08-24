#ifndef NEXT_WORD_H
#define NEXT_WORD_H

#include "ht_item.h"
#include "linked_list.h"

#include <stdint.h>

/*
 * The next_word_t structure represents a word as string of UTF-8 codepoints
 * and its probability as following word respectrfully to the key of the
 * relative ht_item. In this header file, we define the structure and functions
 * to create, manipulate, and print next_word_t objects and define the
 * specialization to create a hash table item (ht_item) that uses next_word_t as
 * value.
 */

typedef struct next_word_t next_word_t;

struct next_word_t {
	int *word;
	double probability;
};

/**
 * @brief Create a next_word_t with probability.
 *
 * @param word UTF-8 code-point array.
 * @param problability Probability value in [0,1].
 * @return Allocated next_word_t or NULL on error.
 */
next_word_t *create_next_word(const int *word, double problability);

/**
 * @brief Create a hash-table item storing followers.
 *
 * @param word Key word.
 * @param next_word Linked list of follower words.
 * @return Allocated item or NULL on error.
 */
ht_item *next_word_ht_item_create(const int *word, linked_list_t *next_word);

/**
 * @brief Free a next_word_t.
 *
 * @param next_word Object to free.
 */
void free_next_word(next_word_t *next_word);

/**
 * @brief Free a hash-table item storing next_word_t values.
 *
 * @param item Item to free.
 */
void free_next_word_ht_item(ht_item *item);

/**
 * @brief Append a next word to an existing item.
 *
 * @param item Hash-table item.
 * @param next_word_to_add next_word_t to add.
 */
void update_ht_item_next_word(const void *item, const void *next_word_to_add);

/**
 * @brief Print a next_word_t.
 *
 * @param next_word Word and probability.
 * @param fd Destination descriptor.
 */
void print_next_word(const next_word_t *next_word, int fd);

/**
 * @brief Print an ht_item containing next words.
 *
 * @param item Hash-table item.
 * @param fd Destination descriptor.
 */
void print_ht_item_next_word(const ht_item *item, int fd);

/**
 * @brief Compare keys for next-word table.
 *
 * @param key1 First key.
 * @param key2 Second key.
 * @return 0 if equal, non-zero otherwise.
 */
int keycmp_next_word(const void *key1, const void *key2);

/**
 * @brief Hash function for next-word keys.
 *
 * @param key Key word.
 * @param table_size Number of buckets.
 * @return Hash index.
 */
uint32_t next_word_hash(const void *key, uint32_t table_size);

/**
 * @brief Build an array of probabilities from an item.
 *
 * @param item Item containing a list of followers.
 * @return Newly allocated array of probabilities.
 */
double *ht_item_next_word_probabilities_array(const ht_item *item);
#endif // NEXT_WORD_H
