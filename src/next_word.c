#include "../include/next_word.h"
#include "../include/utf8_tools.h"
#include "../include/utils.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>


next_word_t *create_next_word(const int *word, double probability) {
  if (word == NULL) {
    fprintf(stderr, "Word is NULL\n");
    return NULL;
  }

  next_word_t *next_word = dmalloc(sizeof(next_word_t));
  int *word_copy = utf8_str_deepcopy(word);
  if (word_copy == NULL) {
    fprintf(stderr, "Failed to create a deep copy of the word\n");
    free(next_word);
    return NULL;
  }
  next_word->word = word_copy;
  next_word->probability = probability;
  return next_word;
}

ht_item *next_word_ht_item_create(const int *key, next_word_t *next_word) {
  if (key== NULL) {
    fprintf(stderr, "Word is NULL\n");
    return NULL;
  }

  if (next_word == NULL) {
    fprintf(stderr, "Failed to create next word\n");
    return NULL;
  }

  ht_item *item = dmalloc(sizeof(ht_item));

  int *key_copy = utf8_str_deepcopy(key);
  if (key_copy == NULL) {
    fprintf(stderr, "Failed to create a deep copy of the word for key\n");
    free_next_word(next_word); // Free the next_word_t if key_copy fails
    free(item);
    return NULL;
  }
  linked_list_t *next_words_list = create_linked_list();
  if (next_words_list == NULL) {
    fprintf(stderr, "Failed to create linked list for next words\n");
    free(key_copy);
    free_next_word(next_word); // Free the next_word_t if list creation fails
    free(item);
    return NULL;
  }
  add_to_list(next_words_list, next_word); // Add the next_word_t to the list

  item->key = key_copy;
  item->value = next_words_list;
  item->update_value = update_ht_item_next_word; // Set the update function
  item->free_item = free_next_word_ht_item;   
  item->print_item = print_ht_item_next_word; 

  return item;
}






void free_next_word(next_word_t *next_word) {
  if (next_word == NULL) {
    return;
  }
  if (next_word->word != NULL) {
    free((void *)next_word->word); // Free the word array
    next_word->word = NULL;        // Set to NULL to avoid dangling pointer
  }
  free(next_word);
}

void free_next_word_ht_item(ht_item *item) {
  if (item == NULL) {
    return;
  }
  if (item->key != NULL) {
    free((void *)item->key); // Free the key word
  }
  if (item->value != NULL) {
    linked_list_t *next_words_list = (linked_list_t *)item->value;
    ll_item_t *current = next_words_list->head;
    while (current != NULL) {
      free_next_word((next_word_t *)current->data); // Free each next_word_t
      current = current->next;
    }
    free(next_words_list); // Free the linked list itself
  }
  free(item);
}

void update_ht_item_next_word(const void *item, const void *next_word_to_add) {
  if (item == NULL || next_word_to_add == NULL) {
    fprintf(stderr, "Item or next word to add is NULL\n");
    return;
  }

  ht_item *ht_item_ptr = (ht_item *)item;
  next_word_t *next_word = (next_word_t *)next_word_to_add;

  if (ht_item_ptr->value == NULL) {
    fprintf(stderr, "Value in hash table item is NULL\n");
    return;
  }

  linked_list_t *next_words_list = (linked_list_t *)ht_item_ptr->value;
  add_to_list(next_words_list, next_word); // Add the next word to the list
}

void print_next_word(const next_word_t *next_word, int fd) {
  if (next_word == NULL || next_word->word == NULL) {
    fprintf(stderr, "Next word or its word is NULL\n");
    return;
  }

  utf8_print_word(next_word->word, fd);
  write(fd, &next_word->probability, sizeof(double)); // Print the probability
}

void print_ht_item_next_word(const ht_item *item, int fd) {
  if (item == NULL || item->key == NULL || item->value == NULL) {
    fprintf(stderr, "Item or its key/value is NULL\n");
    return;
  }

  const int *key = (const int *)item->key;
  linked_list_t *next_words_list = (linked_list_t *)item->value;

  utf8_print_word(key, fd); // Print the key word
  write(fd, "\t", 1); // Print a tab character

  ll_item_t *current = next_words_list->head;
  while (current != NULL) {
    next_word_t *next_word = (next_word_t *)current->data;
    print_next_word(next_word, fd); // Print each next word
    write(fd, ",", sizeof(char)); // Print a comma and space between next words
    current = current->next;
  }
}

int keycmp_next_word(const void *key1, const void *key2) {
  if (key1 == NULL || key2 == NULL) {
    fprintf(stderr, "One or both keys are NULL\n");
    return 0;
  }

  const int *word1 = (const int *)key1;
  const int *word2 = (const int *)key2;

  return utf8_strcmp(word1, word2); // Compare the two words
}

unsigned int next_word_hash(const void *key, int table_size) {
  if (key == NULL) {
    fprintf(stderr, "Key is NULL\n");
    return -1; // Return an invalid index
  }

  const int *word = (const int *)key;
  return hash_function(word, table_size); // Use the hash function defined in utils.h
}



