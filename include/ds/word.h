#ifndef WORD_H
#define WORD_H

#include "ht_item.h"

#include <stdint.h>

/*This structure represents a word and its occurrence count*/

typedef struct {
	int *word; // pointer to the first character(code point) of the word
		   // (integer array)
	int occurrences; // number of occurrences of the word
} word_t;

/**
 * @brief Create a word object from code points.
 *
 * @param word Null-terminated array of code points.
 * @return Allocated word or NULL on error.
 */
word_t *create_word(int *word);

/**
 * @brief Increment the occurrence counter of a word.
 *
 * @param word Word object.
 */
void update_word_occurrences(word_t *word);

/**
 * @brief Get the occurrence count of a word.
 *
 * @param word Word object.
 * @return Number of occurrences.
 */
int get_word_occurrences(const word_t *word);

/**
 * @brief Compare two words.
 *
 * @param word1 First word.
 * @param word2 Second word.
 * @return 0 if equal, non-zero otherwise.
 */
int wordcmp(const word_t *word1, const word_t *word2);

/**
 * @brief Compare a word with a raw string case-insensitively.
 *
 * @param word Word as code points.
 * @param str Null-terminated code-point string.
 * @return Difference as in strcmp.
 */
int word_str_cmp(const int *word, const int *str);

/**
 * @brief Update a hash-table item with a new follower list.
 *
 * @param item Hash table item.
 * @param new_value Linked list of followers.
 */
void update_ht_item_value(const void *item, const void *new_value);

/**
 * @brief Create a hash-table item for a word pair.
 *
 * @param key Current word.
 * @param value Follower word.
 * @return Allocated item or NULL on error.
 */
ht_item *word_ht_item_create(word_t *key, word_t *value);

/**
 * @brief Hash function for word keys.
 *
 * @param key word_t pointer.
 * @param size Table size.
 * @return Hash index.
 */
uint32_t word_hash(const void *key, int size);

/**
 * @brief Print a word in UTF-8.
 *
 * @param word Word object.
 * @param fd Destination descriptor.
 */
void print_utf8_word(const word_t *word, int fd);

/**
 * @brief Print word and optional separator then occurrences.
 *
 * @param word Word object.
 * @param fd Destination descriptor.
 * @param between_char Optional UTF-8 separator.
 */
void word_print(const word_t *word, int fd, int *between_char);

/**
 * @brief Free a word object.
 *
 * @param word Word to free.
 */
void free_word(word_t *word);

/**
 * @brief Free a hash-table item storing words.
 *
 * @param item Item to free.
 */
void ht_item_free_word(ht_item *item);

/**
 * @brief Compare keys for a word hash table.
 *
 * @param key1 First key.
 * @param key2 Second key.
 * @return 0 if equal, non-zero otherwise.
 */
int word_hashtable_keycmp(const void *key1, const void *key2);

/**
 * @brief Print a hash-table item containing words.
 *
 * @param item Item to print.
 * @param fd Destination descriptor.
 */
void word_ht_item_print(const ht_item *item, int fd);

#endif
