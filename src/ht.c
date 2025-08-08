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

hash_table_t *create_hash_table(hash_func_t hash_function,
                                key_compare_func_t key_compare,
                                create_ht_item_func_t create_ht_item) {
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
  ht->hash_func = hash_function;
  ht->key_compare =
      key_compare ? key_compare
                  : default_key_compare; // Use the provided key comparison
  ht->create_ht_item =
      create_ht_item ? create_ht_item
                     : default_create_ht_item; // Use the provided item creation
  // function or default to strcmp

  return ht;
}

static double ht_load_factor(hash_table_t *ht) {
  if (ht == NULL || ht->size == 0)
    return 0.0;
  return (double)ht->count / (double)ht->size;
}

static int calculate_next_ht_size(int old_size) {
  int new_size = old_size * 2;
  return next_prime(new_size);
}

static void free_only_nodes(linked_list_t *list) {
  if (!list)
    return;
  ll_item_t *cur = list->head, *next;
  while (cur) {
    next = cur->next;
    free(cur);
    cur = next;
  }
  free(list);
}

static linked_list_t *check_for_bucket(hash_table_t *ht, unsigned idx) {
  if (ht->buckets[idx] == NULL)
    ht->buckets[idx] = create_linked_list();
  return ht->buckets[idx];
}

static void ht_resize(hash_table_t *ht) {
  if (!ht)
    return;

  int old_size = ht->size;
  linked_list_t **old_buckets = ht->buckets;

  /* 1. dimensione nuova (p.es. primo numero primo ≥ 2×) */
  ht->size = calculate_next_ht_size(old_size);
  ht->buckets = calloc(ht->size, sizeof *ht->buckets);
  if (!ht->buckets) { /* rollback se calloc fallisce */
    perror("calloc");
    ht->buckets = old_buckets;
    ht->size = old_size;
    return;
  }
  for (int i = 0; i < ht->size; ++i)
    ht->buckets[i] = create_linked_list();

  /* 2. reinserisci usando ht_insert (NUOVI nodi, indice corretto) */
  ht->count = 0;
  for (int i = 0; i < old_size; ++i) {
    linked_list_t *bucket = old_buckets[i];
    for (ll_item_t *n = bucket->head; n; n = n->next) {
      ht_item *it = (ht_item *)n->data;
      ht_insert(ht, it->key, it->value);
    }
    free_only_nodes(bucket); /* libera SOLO i nodi vecchi */
  }
  free(old_buckets);
}

void ht_insert(hash_table_t *ht, void *key, void *value) {
  if (ht == NULL || key == NULL)
    return;

  // Check if resizing is needed
  if (ht_load_factor(ht) >= LOAD_FACTOR_THRESHOLD) {
    ht_resize(ht);
  }

  unsigned int index =
      ht->hash_func(key, ht->size); // Calculate the index for the key
  linked_list_t *list = check_for_bucket(ht, index);

  //
  ll_item_t *current = list->head;
  while (current != NULL) {
    ht_item *item = (ht_item *)current->data;
    if (item != NULL && ht->key_compare(item->key, key) == 0) {
      // Key exists, update the value
      item->update_value(item, value);
      return;
    }
    current = current->next;
  }

  // Add a new item in the bucket
  ht_item *new_item = ht->create_ht_item(key, value);
  if (new_item != NULL) {
    add_to_list(list, new_item);
    ht->count++;
  }
}

void *ht_search(hash_table_t *ht, void *key) {
  if (ht == NULL || key == NULL)
    return NULL;

  unsigned int index = ht->hash_func(key, ht->size);
  linked_list_t *list = ht->buckets[index];

  ll_item_t *current = list->head;
  while (current != NULL) {
    ht_item *item = (ht_item *)current->data;
    if (item != NULL && ht->key_compare(item->key, key) == 0) {
      return item->value;
    }
    current = current->next;
  }

  return NULL; // Key not found
}

void ht_delete(hash_table_t *ht, void *key) {
  if (ht == NULL || key == NULL)
    return;

  unsigned int index = ht->hash_func(key, ht->size);
  linked_list_t *list = ht->buckets[index];

  ll_item_t *current = list->head;
  while (current != NULL) {
    ht_item *item = (ht_item *)current->data;
    if (item != NULL && ht->key_compare(item->key, key) == 0) {
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

  // Free each item in the buckets
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

static void free_list_buckets(linked_list_t *list)

{
  ll_item_t *cur = list->head, *next;
  while (cur) {
    next = cur->next;
    free(cur);
    cur = next;
  }
  free(list);
}
