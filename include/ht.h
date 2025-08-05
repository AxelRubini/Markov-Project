#ifndef HT_H 
#define HT_H

#include "ht_item.h"
#include "linked_list.h"

#define START_SIZE 97 // prime number for initial size
#define LOAD_FACTOR_THRESHOLD 0.75

typedef struct {
    linked_list_t **buckets;  // Array of linked lists for buckets 
    int size;                 // actual size of the hash table
    int count;                // Number of elements inserted
    int (*key_compare)(void *key1, void *key2); // Function pointer for key comparison 
} hash_table_t;

// Funzioni principali
hash_table_t *create_hash_table();
void ht_insert(hash_table_t *ht, void *key, void *value,
                void (*update_value)(void *item, void *new_value));
void *ht_search(hash_table_t *ht, void *key);
void ht_delete(hash_table_t *ht, void *key);
void free_hash_table(hash_table_t *ht);

/*This function resizes the hash table it is called when the load factor exceeds the threshold 
input: hash_table_t *ht - the hash table to resize
output: void
*/
void ht_resize(hash_table_t *ht);

/*Calculate the load factor of the hash table,
which is the ratio between the number of elements and the size of the table*/
double ht_load_factor(hash_table_t *ht);

// Funzioni di utilità
int ht_get_count(hash_table_t *ht);
int ht_get_size(hash_table_t *ht);

#endif
