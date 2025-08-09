#include "../include/word.h"
#include "../include/ht_item.h"
#include "../include/linked_list.h"
#include "../include/utf8_tools.h"
#include "../include/utils.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

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
  item->key = word_deep_copy(key); // Create a deep copy of the key
  item->update_value = update_ht_item_value; // Set the update function
  item->value = create_linked_list(); // Initialize value to the word_t pointer
  add_to_list((linked_list_t *)item->value,
              word_deep_copy(value)); // Add the value to the linked list
  item->free_item = ht_item_free_word; // Set the free function to free_word

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
    if (!word || !str) return 0;

    int *lower_word = utf8_word_to_lower(word, MAX_WORD_LENGTH);
    int *lower_str  = utf8_word_to_lower(str,  MAX_WORD_LENGTH);
    if (!lower_word || !lower_str) { /* gestione minima errore */
        if (lower_word) free(lower_word);
        if (lower_str)  free(lower_str);
        return 0;
    }

    int i = 0;
    while (lower_word[i] != 0 && lower_str[i] != 0) {
        if (lower_word[i] != lower_str[i]) {
            int diff = lower_word[i] - lower_str[i];
            free(lower_word);
            free(lower_str);
            return diff;
        }
        i++;
    }

    int diff = lower_word[i] - lower_str[i];  /* include il caso del terminatore */
    free(lower_word);
    free(lower_str);
    return diff;
}

static int *update_value_helper(const ht_item *item){
  if (item == NULL || item->value == NULL) {
    fprintf(stderr, "Item or value is NULL\n");
    return NULL;
  }
  linked_list_t *list = (linked_list_t *)item->value;
  ll_item_t *current = list->head;
  if (current == NULL) {
    fprintf(stderr, "List is empty\n");
    return NULL;
  }
  word_t *word = (word_t *)current->data;
  return word->word; // Return the word array from the first item in the list

}

void update_ht_item_value(const void *item, const void *actual_followers) {
  if (item == NULL || actual_followers == NULL) {
    fprintf(stderr, "Item or new value is NULL\n");
    return;
  }
  ht_item *htItem = (ht_item *)item;
  linked_list_t *af_list = (linked_list_t *)actual_followers;
  ll_item_t *current = af_list->head;
  int *nw = update_value_helper(htItem); 
  int *lower_nw = utf8_word_to_lower(nw, MAX_WORD_LENGTH);
  while (current != NULL) {
    word_t *word = (word_t *)current->data;
    if (word_str_cmp(word->word, lower_nw) == 0) {
      // If the word already exists, update its occurrences
      update_word_occurrences(word);
      free(lower_nw); // Free the lower_nw array
      return;
    }
    current = current->next;
  }
  free(lower_nw); // Free the lower_nw array
  // If the word does not exist, add it to the linked list
  word_t *new_word = create_word(nw);
  add_to_list(af_list, new_word);
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
    word->word = NULL; // Set to NULL to avoid dangling pointer
  }

  free(word);
}

void ht_item_free_word(ht_item *item) {
  if (item == NULL) {
    return;
  }
  if (item->key != NULL) {
    free_word((word_t *)item->key); // Free the key word
  }
  if (item->value != NULL) {
    word_followers_free_list((linked_list_t *) item->value); // Free the linked list
  }
  free(item);
}

static void word_followers_free_list(linked_list_t *list) {
  if (list == NULL) {
    fprintf(stderr, "List is NULL\n");
    return;
  }
  ll_item_t *current = list->head;
  while (current != NULL) {
    ll_item_t *next = current->next;
    free_word((word_t *)current->data); // Free each word in the list
    free(current);
    current = next;
  }
  free(list);
}

int word_hashtable_keycmp(const void *key1, const void *key2) {
  if (key1 == NULL || key2 == NULL) {
    fprintf(stderr, "One or both keys are NULL\n");
    return 0;
  }
  return wordcmp((const word_t *)key1, (const word_t *)key2);
}

static word_t *word_deep_copy(const word_t *original) {
  if (!original || !original->word) {
  return NULL;
  }
  size_t len = 0;
  while (len < MAX_WORD_LENGTH && original->word[len] != '\0') {
    ++len;
  }

  word_t *copy = dmalloc(sizeof(word_t));
  copy->word = dmalloc((len + 1) * sizeof(int));
  if (len) memcpy(copy->word, original->word, len * sizeof(int));
  copy->word[len] = '\0'; 
  copy->occurrences = original->occurrences;
  return copy;

}

