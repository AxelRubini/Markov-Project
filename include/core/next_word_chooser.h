#ifndef NEXT_WORD_CHOOSER_H
#define NEXT_WORD_CHOOSER_H

#include "hash_table.h"

typedef struct {
	hash_table_t *table; // Hash table containing next words
} next_word_chooser_t;

/**
 * @brief Create a chooser for next-word selection.
 *
 * @param table Hash table containing transitions.
 * @return Allocated chooser or NULL on error.
 */
next_word_chooser_t *create_next_word_chooser(hash_table_t *table);

/**
 * @brief Pick the next word according to probabilities.
 *
 * @param chooser Initialized chooser.
 * @param current_word Current word as code points.
 * @return Newly allocated next word or NULL if unavailable.
 */
int *next_word_chooser_choose(next_word_chooser_t *chooser,
			      const int *current_word);

/**
 * @brief Free a next word chooser.
 *
 * @param chooser Instance to release.
 */
void next_word_chooser_free(next_word_chooser_t *chooser);

#endif // NEXT_WORD_CHOOSER_H
