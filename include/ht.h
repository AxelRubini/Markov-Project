#ifndef HT_H
#define HT_H

#include "ht_item.h"
#include "linked_list.h"

#define START_SIZE 97 // prime number for initial size
#define LOAD_FACTOR_THRESHOLD 0.75

// Type for hash function of the hash table
typedef unsigned int (*hash_func_t)(void *key, int size);

// Type for the function that creates a hash table item
typedef ht_item *(*create_ht_item_func_t)(void *key, void *value);

// Type for the function that compares keys in the hash table
typedef int (*key_compare_func_t)(void *key1, void *key2);

typedef struct {

  linked_list_t **buckets;        // Array of linked lists for buckets
  int size;                       // actual size of the hash table
  int count;                      // Number of elements inserted
  hash_func_t hash_func;          // Function to calculate the hash
  key_compare_func_t key_compare; // Function to compare keys
  create_ht_item_func_t create_ht_item; // Function to create hash table items
} hash_table_t;

// Funzioni principali
hash_table_t *create_hash_table(hash_func_t hash_func,
                                key_compare_func_t key_compare,
                                create_ht_item_func_t create_ht_item);
void ht_insert(hash_table_t *ht, void *key, void *value);
void *ht_search(hash_table_t *ht, void *key);
void ht_delete(hash_table_t *ht, void *key);
void free_hash_table(hash_table_t *ht);

/*This function resizes the hash table it is called when the load factor exceeds
the threshold input: hash_table_t *ht - the hash table to resize output: void
*/
static void ht_resize(hash_table_t *ht);

/*Calculate the load factor of the hash table,
which is the ratio between the number of elements and the size of the table*/
static double ht_load_factor(hash_table_t *ht);

// Funzioni di utilità
int ht_get_count(hash_table_t *ht);
int ht_get_size(hash_table_t *ht);

static void free_list_buckets(linked_list_t *list); 

void debug_hash_table(hash_table_t *ht, const char *moment);

#endif
