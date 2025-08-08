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

/* ---------------------------------------------------------------------------
 *  Resize the hash table WITHOUT calling ht_insert (O(N) rehash).
 * ------------------------------------------------------------------------- */

/* ---------------------------------------------------------------------------
 *  Resize the hash table WITHOUT calling ht_insert (O(N) rehash).
 * ------------------------------------------------------------------------- */

static void ht_resize(hash_table_t *ht) {
  if (ht == NULL)
    return;

  printf("DEBUG: Starting resize - old_size=%d, count=%d\n", ht->size,
         ht->count);

  // Prima verifica che k42 ci sia
  void *test_val = ht_search(ht, "k42");
  printf("DEBUG: Before ANY operation - k42 found: %s\n",
         test_val ? "YES" : "NO");

  // Salva tutto lo stato vecchio
  int old_size = ht->size;
  linked_list_t **old_buckets = ht->buckets;

  // Calcola nuova dimensione
  int new_size = calculate_next_ht_size(old_size);
  printf("DEBUG: new_size calculated: %d\n", new_size);

  // Alloca nuovi bucket
  linked_list_t **new_buckets = dmalloc(sizeof(linked_list_t *) * new_size);
  if (!new_buckets)
    return;

  // Inizializza nuovi bucket
  for (int i = 0; i < new_size; i++) {
    new_buckets[i] = create_linked_list();
    if (!new_buckets[i]) {
      for (int j = 0; j < i; j++) {
        free_linked_list(new_buckets[j]);
      }
      free(new_buckets);
      return;
    }
  }

  // Verifica che k42 ci sia ancora (dovrebbe essere SI)
  test_val = ht_search(ht, "k42");
  printf("DEBUG: After creating new buckets - k42 found: %s\n",
         test_val ? "YES" : "NO");

  // Conta e rehash elementi
  int items_rehashed = 0;
  for (int i = 0; i < old_size; i++) {
    if (old_buckets[i]) {
      ll_item_t *node = old_buckets[i]->head;
      while (node) {
        ht_item *item = (ht_item *)node->data;
        if (item) {
          unsigned int new_idx = ht->hash_func(item->key, new_size);

          // Debug per k42
          if (ht->key_compare(item->key, "k42") == 0) {
            printf("DEBUG: Moving k42 from bucket %d to bucket %u, value=%d\n",
                   i, new_idx, *(int *)item->value);
          }

          add_to_list(new_buckets[new_idx], item);
          items_rehashed++;
        }
        node = node->next;
      }
    }
  }

  printf("DEBUG: Rehashed %d items\n", items_rehashed);

  // Free old structure
  for (int i = 0; i < old_size; i++) {
    if (old_buckets[i]) {
      ll_item_t *node = old_buckets[i]->head;
      while (node) {
        ll_item_t *next = node->next;
        free(node);
        node = next;
      }
      free(old_buckets[i]);
    }
  }
  free(old_buckets);

  // SOLO ADESSO aggiorna la hash table
  ht->buckets = new_buckets;
  ht->size = new_size;

  // Verifica finale
  test_val = ht_search(ht, "k42");
  printf("DEBUG: After updating ht structure - k42 found: %s\n",
         test_val ? "YES" : "NO");
  if (test_val) {
    printf("DEBUG: k42 value: %d\n", *(int *)test_val);
  }
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

  // CORREZIONE: Controllo se la lista esiste
  if (list == NULL)
    return NULL;

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

  // CORREZIONE: Controllo se la lista esiste
  if (list == NULL)
    return;

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
void *ht_search_debug(hash_table_t *ht, void *key) {
  printf("DEBUG ht_search: searching for key '%s'\n", (char *)key);

  if (ht == NULL || key == NULL) {
    printf("DEBUG ht_search: ht or key is NULL\n");
    return NULL;
  }

  printf("DEBUG ht_search: ht->size=%d, ht->count=%d\n", ht->size, ht->count);

  unsigned int index = ht->hash_func(key, ht->size);
  printf("DEBUG ht_search: calculated index=%u\n", index);

  if (index >= ht->size) {
    printf("DEBUG ht_search: ERROR - index %u >= size %d\n", index, ht->size);
    return NULL;
  }

  linked_list_t *list = ht->buckets[index];
  printf("DEBUG ht_search: bucket[%u] = %p\n", index, (void *)list);

  if (list == NULL) {
    printf("DEBUG ht_search: bucket is NULL\n");
    return NULL;
  }

  printf("DEBUG ht_search: list->head = %p\n", (void *)list->head);

  ll_item_t *current = list->head;
  int node_count = 0;
  while (current != NULL) {
    printf("DEBUG ht_search: examining node %d at %p\n", node_count,
           (void *)current);

    ht_item *item = (ht_item *)current->data;
    printf("DEBUG ht_search: item = %p\n", (void *)item);

    if (item == NULL) {
      printf("DEBUG ht_search: item is NULL, skipping\n");
      current = current->next;
      node_count++;
      continue;
    }

    printf("DEBUG ht_search: item->key = %p ('%s')\n", item->key,
           (char *)item->key);
    printf("DEBUG ht_search: item->value = %p\n", item->value);

    if (item->value) {
      printf("DEBUG ht_search: item->value = %d\n", *(int *)item->value);
    }

    int cmp_result = ht->key_compare(item->key, key);
    printf("DEBUG ht_search: key_compare result = %d\n", cmp_result);

    if (cmp_result == 0) {
      printf("DEBUG ht_search: FOUND! returning %p\n", item->value);
      return item->value;
    }

    current = current->next;
    node_count++;
  }

  printf("DEBUG ht_search: key not found after examining %d nodes\n",
         node_count);
  return NULL; // Key not found
}

// Funzione per debuggare l'intera hash table
void debug_hash_table(hash_table_t *ht, const char *moment) {
  printf("DEBUG HT [%s]: size=%d, count=%d\n", moment, ht->size, ht->count);

  int total_items = 0;
  for (int i = 0; i < ht->size; i++) {
    if (ht->buckets[i] && ht->buckets[i]->head) {
      printf("DEBUG HT [%s]: bucket[%d] has items:\n", moment, i);

      ll_item_t *current = ht->buckets[i]->head;
      while (current) {
        ht_item *item = (ht_item *)current->data;
        if (item) {
          printf("  - key='%s' value=%d\n", (char *)item->key,
                 item->value ? *(int *)item->value : -999);
          total_items++;

          // Verifica speciale per k42
          if (strcmp((char *)item->key, "k42") == 0) {
            printf("  *** FOUND k42 in bucket %d! ***\n", i);
          }
        }
        current = current->next;
      }
    }
  }
  printf("DEBUG HT [%s]: total items found: %d\n", moment, total_items);
}

// Versione di resize con debug completo
static void ht_resize_full_debug(hash_table_t *ht) {
  if (ht == NULL)
    return;

  printf("\n=== RESIZE DEBUG START ===\n");
  debug_hash_table(ht, "BEFORE_RESIZE");

  // Test search prima di tutto
  void *test_val = ht_search_debug(ht, "k42");
  printf("Direct search result: %p\n", test_val);

  // Il resto del resize normale...
  int old_size = ht->size;
  linked_list_t **old_buckets = ht->buckets;
  int new_size = calculate_next_ht_size(old_size);

  linked_list_t **new_buckets = dmalloc(sizeof(linked_list_t *) * new_size);
  if (!new_buckets)
    return;

  for (int i = 0; i < new_size; i++) {
    new_buckets[i] = create_linked_list();
    if (!new_buckets[i]) {
      for (int j = 0; j < i; j++) {
        free_linked_list(new_buckets[j]);
      }
      free(new_buckets);
      return;
    }
  }

  // Rehash
  for (int i = 0; i < old_size; i++) {
    if (old_buckets[i]) {
      ll_item_t *node = old_buckets[i]->head;
      while (node) {
        ht_item *item = (ht_item *)node->data;
        if (item) {
          unsigned int new_idx = ht->hash_func(item->key, new_size);
          add_to_list(new_buckets[new_idx], item);
        }
        node = node->next;
      }
    }
  }

  // Free old
  for (int i = 0; i < old_size; i++) {
    if (old_buckets[i]) {
      ll_item_t *node = old_buckets[i]->head;
      while (node) {
        ll_item_t *next = node->next;
        free(node);
        node = next;
      }
      free(old_buckets[i]);
    }
  }
  free(old_buckets);

  // Update
  ht->buckets = new_buckets;
  ht->size = new_size;

  debug_hash_table(ht, "AFTER_RESIZE");
  printf("=== RESIZE DEBUG END ===\n\n");
}
