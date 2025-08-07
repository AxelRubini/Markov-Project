#include "../include/word.h"
#include "../include/ht_item.h"
#include "../include/linked_list.h"
#include "../include/utf8_tools.h"
#include "../include/utils.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

word_t *create_word(int *word) {
  if (word == NULL) {
    fprintf(stderr, "Word is NULL\n");
    return NULL;
  }

  word_t *new_word = dmalloc(sizeof(word_t));

  new_word->word = utf8_word_to_lower(word, MAX_WORD_LENGTH);
  new_word->occurrences = 1;

  return new_word;
}

ht_item *word_ht_item_create(word_t *key, word_t *value) {
  if (key == NULL || value == NULL) {
    fprintf(stderr, "Key or value is NULL\n");
    return NULL;
  }

  ht_item *item = dmalloc(sizeof(ht_item));
  item->key = key;
  item->update_value = update_ht_item_value; // Set the update function
  item->value = create_linked_list(); // Initialize value to the word_t pointer
  add_to_list((linked_list_t *)item->value,
              value); // Add the value to the linked list

  return item;
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

int wordcmp(const word_t *word1, const word_t *word2) {
  if (word1 == NULL || word2 == NULL) {
    fprintf(stderr, "One or both words are NULL\n");
    return 0;
  }

  int i = 0;
  while (word1->word[i] != '\0' && word2->word[i] != '\0') {
    if (word1->word[i] != word2->word[i]) {
      return word1->word[i] - word2->word[i];
    }
    i++;
  }
  return word1->word[i] - word2->word[i];
}

int word_str_cmp(const int *word, const int *str) {
  if (word == NULL || str == NULL) {
    fprintf(stderr, "One or both words are NULL\n");
    return 0;
  }

  int i = 0;
  while (word[i] != '\0' && str[i] != '\0') {
    if (word[i] != str[i]) {
      return word[i] - str[i];
    }
    i++;
  }
  return word[i] - str[i];
}

void update_ht_item_value(const void *item, const void *new_value) {
  if (item == NULL || new_value == NULL) {
    fprintf(stderr, "Item or new value is NULL\n");
    return;
  }
  ht_item *htItem = (ht_item *)item;
  linked_list_t *list = (linked_list_t *)htItem->value;
  ll_item_t *current = list->head;
  int *nw = (int *)new_value;
  while (current != NULL) {
    word_t *word = (word_t *)current->data;
    if (word_str_cmp(word->word, nw) == 0) {
      // If the word already exists, update its occurrences
      update_word_occurrences(word);
      return;
    }
    current = current->next;
  }
  // If the word does not exist, add it to the linked list
  word_t *new_word = create_word(nw);
  add_to_list(list, new_word);
}

unsigned int word_hash(const void *key, int size) {
  if (key == NULL) {
    fprintf(stderr, "Key is NULL\n");
    return 0;
  }
  const word_t *word = (const word_t *)key;
  return hash_function(word->word,
                       size); // Use the hash function on the word array
}

void print_utf8_word(const word_t *word, int fd) {
  if (word == NULL) {
    fprintf(stderr, "Word is NULL\n");
    return;
  }
  utf8_print_word(word->word, fd);
}

void word_print(const word_t *word, int fd, int *between_char) {
  if (word == NULL) {
    fprintf(stderr, "Word is NULL\n");
    return;
  }
  int occurrences = get_word_occurrences(word);
  if (between_char != NULL) {
    print_utf8_word(word, fd);
    for (int i = 0; between_char[i] != '\0'; i++) {
      utf8_putchar(between_char[i], fd); // Print the character between words
    }
    write(fd, &occurrences, sizeof(int)); // Write the occurrences of the word
    return;                               // Print the occurrences of the word
  }
  print_utf8_word(word, fd);
  write(fd, &occurrences, sizeof(int)); // Write the occurrences of the word
}

void free_word(word_t *word) {
  if (word == NULL) {
    return;
  }
  if (word->word != NULL) {
    free((void *)word->word); // Free the word array
  }

  free(word);
}
