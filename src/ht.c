#include "../include/ht.h"
#include "../include/utils.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static int default_key_compare(void *key1, void *key2) {
  if (key1 == NULL || key2 == NULL) {
    fprintf(stderr, "One or both keys are NULL\n");
    return 0;
  }
  return strcmp((char *)key1, (char *)key2);
}

hash_table_t *create_hash_table(unsigned int (*hash_func)(void *, int),
                                int (*key_compare)(void *, void *)) {
  hash_table_t *ht = dmalloc(sizeof(hash_table_t));

  ht->size = START_SIZE;
  ht->count = 0;

  // Build array of Buckets
  ht->buckets = dmalloc(sizeof(linked_list_t *) * ht->size);

  // Initialize each bucket with an empty linked list
  for (int i = 0; i < ht->size; i++) {
    ht->buckets[i] = create_linked_list();
    if (ht->buckets[i] == NULL) {
      // de-allocate in case of error
      for (int j = 0; j < i; j++) {
        free_linked_list(ht->buckets[j]);
      }
      free(ht->buckets);
      free(ht);
      return NULL;
    }
  }
  ht->key_compare =
      key_compare ? key_compare
                  : default_key_compare; // Use the provided key comparison
                                         // function or default to strcmp

  return ht;
}

double ht_load_factor(hash_table_t *ht) {
  if (ht == NULL || ht->size == 0)
    return 0.0;
  return (double)ht->count / (double)ht->size;
}

int calculate_next_ht_size(int old_size) {
  int new_size = old_size * 2;
  return next_prime(new_size);
}

void ht_resize(hash_table_t *ht) {
  if (ht == NULL)
    return;

  // Save the old-buckets to copy them later
  linked_list_t **old_buckets = ht->buckets;
  int old_size = ht->size;

  // look for the next size min 2* old_size
  ht->size = calculate_next_ht_size(old_size);
  ht->count = 0;

  // assign new buckets slot
  ht->buckets = malloc(sizeof(linked_list_t *) * ht->size);
  if (ht->buckets == NULL) { // if allocation fails then rollback to old buckets
    fprintf(stderr, "Memory allocation failed during resize\n");
    ht->buckets = old_buckets;
    ht->size = old_size;
    return;
  }

  // Initialize new buckets
  for (int i = 0; i < ht->size; i++) {
    ht->buckets[i] = create_linked_list();
    if (ht->buckets[i] == NULL) {
      fprintf(stderr, "Failed to create linked list during resize\n");
      // Cleanup & rollback
      for (int j = 0; j < i; j++) {
        free_linked_list(ht->buckets[j]);
      }
      free(ht->buckets);
      ht->buckets = old_buckets;
      ht->size = old_size;
      return;
    }
  }

  // Copy the old items from old buckets to new buckets
  for (int i = 0; i < old_size; i++) {
    linked_list_t *list = old_buckets[i];
    if (list->head != NULL) {
      ll_item_t *current = list->head;
      while (current != NULL) {
        ht_item *item = (ht_item *)current->data;
        if (item != NULL) {
          // Recalculate the index for the new table size
          unsigned int index = hash_function(item->key, ht->size);
          add_to_list(ht->buckets[index], item);
          ht->count++;
        }
        current = current->next;
      }
    }
  }

  // Free the old buckets
  for (int i = 0; i < old_size; i++) {
    free_linked_list(old_buckets[i]);
  }
  free(old_buckets);
}

void ht_insert(hash_table_t *ht, void *key, void *value,
               void (*update_value)(void *item, void *new_value)) {
  if (ht == NULL || key == NULL)
    return;

  // Check if resizing is needed
  if (ht_load_factor(ht) >= LOAD_FACTOR_THRESHOLD) {
    ht_resize(ht);
  }

  unsigned int index =
      hash_function(key, ht->size); // Calculate the index for the key
  linked_list_t *list = ht->buckets[index];

  //
  ll_item_t *current = list->head;
  while (current != NULL) {
    ht_item *item = (ht_item *)current->data;
    if (item != NULL && ht->key_compare(item->key,key) == 0) {
      // Aggiorna il valore esistente
      item->value = value;
      return;
    }
    current = current->next;
  }

  // Crea nuovo item e inseriscilo
  ht_item *new_item = create_ht_item(key, value, update_value);
  if (new_item != NULL) {
    add_to_list(list, new_item);
    ht->count++;
  }
}

void *ht_search(hash_table_t *ht, void *key) {
  if (ht == NULL || key == NULL)
    return NULL;

  unsigned int index = hash_function(key, ht->size);
  linked_list_t *list = ht->buckets[index];

  ll_item_t *current = list->head;
  while (current != NULL) {
    ht_item *item = (ht_item *)current->data;
    if (item != NULL && ht->key_compare(item->key,key) == 0) {
      return item->value;
    }
    current = current->next;
  }

  return NULL; // Non trovato
}

void ht_delete(hash_table_t *ht, void *key) {
  if (ht == NULL || key == NULL)
    return;

  unsigned int index = hash_function(key, ht->size);
  linked_list_t *list = ht->buckets[index];

  ll_item_t *current = list->head;
  while (current != NULL) {
    ht_item *item = (ht_item *)current->data;
    if (item != NULL && strcmp((char *)item->key, (char *)key) == 0) {
      remove_from_list(list, item);
      free_ht_item(item);
      ht->count--;
      return;
    }
    current = current->next;
  }
}

void free_hash_table(hash_table_t *ht) {
  if (ht == NULL)
    return;

  // Libera tutti gli item in tutti i buckets
  for (int i = 0; i < ht->size; i++) {
    if (ht->buckets[i] != NULL) {
      ll_item_t *current = ht->buckets[i]->head;
      while (current != NULL) {
        ht_item *item = (ht_item *)current->data;
        if (item != NULL) {
          free_ht_item(item);
        }
        current = current->next;
      }
      free_linked_list(ht->buckets[i]);
    }
  }

  free(ht->buckets);
  free(ht);
}

int ht_get_count(hash_table_t *ht) { return ht ? ht->count : 0; }

int ht_get_size(hash_table_t *ht) { return ht ? ht->size : 0; }
