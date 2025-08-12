#ifndef NEXT_WORD_CHOOSER_H
#define NEXT_WORD_CHOOSER_H
#include "next_word.h"
#include "ht_item.h"
#include "linked_list.h"
#include "hash_table.h"


#include <gsl/gsl_rng.h>
#include <gsl/gsl_randist.h>
#include <time.h>
#include <stdio.h>


typedef struct {
  hash_table_t *table; // Hash table containing next words
  gsl_rng *rng;        // Random number generator
} next_word_chooser_t;

/**
 * Initializes the next word chooser with a hash table and a random number generator.
 * @param table The hash table containing next words.
 * @return A pointer to the initialized next_word_chooser_t.
 */
next_word_chooser_t *create_next_word_chooser(hash_table_t *table);
/**
 * Chooses the next word based on the current word and the probabilities of next words.
 * @param chooser The next word chooser.
 * @param current_word The current word as an array of UTF-8 code points.
 * @return A pointer to the chosen next_word_t, or NULL if no next word is found.
 */
int *next_word_chooser_choose(next_word_chooser_t *chooser, const int *current_word);
/**
 * Frees the resources used by the next word chooser.
 * @param chooser The next word chooser to free.
 */
void next_word_chooser_free(next_word_chooser_t *chooser);

#endif // NEXT_WORD_CHOOSER_H
