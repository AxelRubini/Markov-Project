#ifndef HASH_TABLE_H
#define HASH_TABLE_H

#include "ht_item.h"
#include "linked_list.h"

#define HASH_TABLE_SIZE 97 // Initial size of the hash table
#define LOAD_FACTOR_THRESHOLD 0.75

typedef struct {
  linked_list_t **buckets; /* array of bucket lists of ht_item* */
  int size;                /* current number of buckets */
  int count;               /* number of stored items  */
  unsigned int (*hash_func)(const void *key, int size); /* key -> hash */
  int (*key_cmp)(const void *key1, const void *key2);   /* key compare */
} hash_table_t;

/**
 * @brief Create a hash table.
 *
 * @param initial_size Desired initial bucket count.
 * @param hash_func Hash function.
 * @param key_cmp Key comparison function.
 * @return Newly allocated table or NULL on error.
 */
hash_table_t *create_hash_table(int initial_size,
                                unsigned int (*hash_func)(const void *, int),
                                int (*key_cmp)(const void *, const void *));

/**
 * @brief Insert an item into the table.
 *
 * @param table Hash table.
 * @param item Item to insert.
 */
void ht_insert(hash_table_t *table, ht_item *item);

/**
 * @brief Search an item by key.
 *
 * @param table Hash table.
 * @param key Key to search for.
 * @return Matching item or NULL if not found.
 */
ht_item *ht_search(const hash_table_t *table, const void *key);

/**
 * @brief Remove an item by key.
 *
 * @param table Hash table.
 * @param key Key to remove.
 * @return 1 if removed, 0 if not found.
 */
int ht_remove(hash_table_t *table, const void *key);

/**
 * @brief Destroy a hash table and its contents.
 *
 * @param table Table to free.
 */
void free_hash_table(hash_table_t *table);

/**
 * @brief Get the number of stored items.
 *
 * @param table Hash table.
 * @return Item count.
 */
int ht_get_count(const hash_table_t *table);

/**
 * @brief Get the current number of buckets.
 *
 * @param table Hash table.
 * @return Bucket count.
 */
int ht_get_size(const hash_table_t *table);

/**
 * @brief Print all items using their callbacks.
 *
 * @param table Hash table.
 * @param fd Destination descriptor.
 */
void print_hash_table(const hash_table_t *table, int fd);


#endif /* HASH_TABLE_H */
