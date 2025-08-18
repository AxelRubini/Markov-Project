#include "next_word_chooser.h"
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "utils.h"
#include "utf8_tools.h"
#include "next_word.h"

next_word_chooser_t *create_next_word_chooser(hash_table_t *table) {
  if (table == NULL) {
    fprintf(stderr, "Hash table is NULL\n");
    return NULL;
  }

  next_word_chooser_t *chooser =
      (next_word_chooser_t *)dmalloc(sizeof(next_word_chooser_t));
  chooser->table = table;

  // Initialize the random number generator
  srand((unsigned int)time(NULL));

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
    printf("\n");
    fflush(stdout);
    utf8_print_word(current_word, 1);
    printf("\n");
    fflush(stdout);
    print_ht_item_next_word(item, 1);
    fprintf(stderr, "No next words found for the current word\n");
    return NULL;
  }
  int num_next_words = get_list_size((linked_list_t *)item->value);
  double r = (double)rand() / RAND_MAX;
  double cumulative = 0.0;
  int chosen_index = 0;
  for (int i = 0; i < num_next_words; i++) {
    cumulative += probabilities[i];
    if (r <= cumulative) {
      chosen_index = i;
      break;
    }
  }
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
  return chosen_word;
}
void next_word_chooser_free(next_word_chooser_t *chooser) {
  if (chooser == NULL) {
    return;
  }
  free(chooser); // Free the chooser structure
}
