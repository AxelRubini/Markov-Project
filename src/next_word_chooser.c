#include "../include/next_word_chooser.h"
#include <stdio.h>

next_word_chooser_t *create_next_word_chooser(hash_table_t *table) {
  if (table == NULL) {
    fprintf(stderr, "Hash table is NULL\n");
    return NULL;
  }

  next_word_chooser_t *chooser =
      (next_word_chooser_t *)dmalloc(sizeof(next_word_chooser_t));
  chooser->table = table;

  // Initialize the random number generator
  gsl_rng_env_setup();
  chooser->rng = gsl_rng_alloc(gsl_rng_default);
  gsl_rng_set(chooser->rng, time(NULL)); // Seed with current time

  return chooser;
}

/**
 * Not an optimal implementation, need to build a chache to improve performance
 */
int *next_word_chooser_choose(next_word_chooser_t *chooser,
                              const int *current_word) {
  if (chooser == NULL || chooser->table == NULL || current_word == NULL) {
    fprintf(stderr, "Invalid arguments to next_word_chooser_choose\n");
    return NULL;
  }

  ht_item *item = ht_search(chooser->table, current_word);
  double *probabilities = ht_item_next_word_probabilities_array(item);
  if (probabilities == NULL) {
    fprintf(stderr, "No next words found for the current word\n");
    return NULL;
  }
  int num_next_words = get_list_size((linked_list_t *)item->value);
  gsl_ran_discrete_t *distribution =
      gsl_ran_discrete_preproc(num_next_words, probabilities);
  if (distribution == NULL) {
    fprintf(stderr, "Failed to create discrete distribution\n");
    free(probabilities);
    return NULL;
  }
  int chosen_index = gsl_ran_discrete(chooser->rng, distribution);
  gsl_ran_discrete_free(distribution);
  free(probabilities);
  ll_item_t *current = ((linked_list_t *)item->value)->head;
  for (int i = 0; i < chosen_index && current != NULL; i++) {
    current = current->next;
  }
  if (current == NULL) {
    fprintf(stderr, "No next word found at index %d\n", chosen_index);
    return NULL;
  }
  next_word_t *next_word = (next_word_t *)current->data;
  int *chosen_word = utf8_str_deepcopy(next_word->word);
  if (chosen_word == NULL) {
    fprintf(stderr, "Failed to create a deep copy of the chosen word\n");
    return NULL;
  }
  gsl_rng_free(chooser->rng); // Free the random number generator
  return chosen_word;         // Return the deep copy of the chosen word
}
void next_word_chooser_free(next_word_chooser_t *chooser) {
  if (chooser == NULL) {
    return;
  }
  if (chooser->rng != NULL) {
    gsl_rng_free(chooser->rng); // Free the random number generator
  }
  free(chooser); // Free the chooser structure
}
