/* ================= hash_table.h ================= */
#ifndef HASH_TABLE_H
#define HASH_TABLE_H

#include "ht_item.h"
#include "linked_list.h"

/* Generic separate‑chaining hash table. */
typedef struct {
  linked_list_t **buckets; /* array of bucket lists of ht_item* */
  int size;                /* current number of buckets */
  int count;               /* number of stored items  */
  unsigned int (*hash_func)(const void *key, int size); /* key -> hash */
  int (*key_cmp)(const void *key1, const void *key2);   /* key compare */
} hash_table_t;

/* Allocate a new table; initial_size is rounded up to the next prime. */
hash_table_t *create_hash_table(int initial_size,
                                unsigned int (*hash_func)(const void *, int),
                                int (*key_cmp)(const void *, const void *));

/* Insert a new item. If the key already exists, the stored
 * ht_item->update_value function is invoked and the *item* passed to ht_insert
 * is freed. */
void ht_insert(hash_table_t *table, ht_item *item);

/* Search an item by key; returns NULL if not found. */
ht_item *ht_search(const hash_table_t *table, const void *key);

/* Remove an item and optionally free it (set free_item to NULL to skip). */
int ht_remove(hash_table_t *table, const void *key,
              void (*free_item)(ht_item *));

/* Destroy the entire table; free_item is applied to every stored item. */
void free_hash_table(hash_table_t *table, void (*free_item)(ht_item *));

int ht_get_count(const hash_table_t *table);
int ht_get_size(const hash_table_t *table);

#endif /* HASH_TABLE_H */
