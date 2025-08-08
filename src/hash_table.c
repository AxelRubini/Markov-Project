#include "../include/hash_table.h"
#include "../include/utils.h"
#include <stdio.h>
#include <stdlib.h>

#define LOAD_FACTOR_THRESHOLD 0.75

/* Ensure the bucket at index exists, creating a linked list if necessary. */
static linked_list_t *ensure_bucket(hash_table_t *table, int index) {
    if (table->buckets[index] == NULL) {
        table->buckets[index] = create_linked_list();
    }
    return table->buckets[index];
}

/* Resize the table when the load factor exceeds the threshold. */
static void ht_resize(hash_table_t *table) {
    int new_size = next_prime(table->size * 2);
    linked_list_t **old_buckets = table->buckets;
    int old_size = table->size;

    table->buckets = dmalloc(sizeof(linked_list_t *) * new_size);
    table->size = new_size;
    for (int i = 0; i < new_size; ++i) {
        table->buckets[i] = NULL;
    }

    /* Re‑hash every element into the new bucket array. */
    for (int i = 0; i < old_size; ++i) {
        linked_list_t *bucket = old_buckets[i];
        if (bucket == NULL) continue;

        ll_item_t *current = bucket->head;
        while (current) {
            ht_item *it = (ht_item *)current->data;
            int index = table->hash_func(get_ht_item_key(it), table->size);
            linked_list_t *dest = ensure_bucket(table, index);
            add_to_list(dest, it);
            current = current->next;
        }
        /* Free the wrapper list; items are now in the new table. */
        free(bucket);
    }
    free(old_buckets);
}

hash_table_t *create_hash_table(int initial_size,
                                unsigned int (*hash_func)(const void *, int),
                                int (*key_cmp)(const void *, const void *)) {
    if (!hash_func || !key_cmp) {
        fprintf(stderr, "Hash function and key compare cannot be NULL\n");
        return NULL;
    }
    hash_table_t *table = dmalloc(sizeof(hash_table_t));
    table->size  = next_prime(initial_size);
    table->count = 0;
    table->hash_func = hash_func;
    table->key_cmp   = key_cmp;

    table->buckets = dmalloc(sizeof(linked_list_t *) * table->size);
    for (int i = 0; i < table->size; ++i) {
        table->buckets[i] = NULL;
    }
    return table;
}

static double load_factor(const hash_table_t *table) {
    return (double)table->count / (double)table->size;
}

void ht_insert(hash_table_t *table, ht_item *item) {
    if (!table || !item) return;

    int index = table->hash_func(get_ht_item_key(item), table->size);
    linked_list_t *bucket = ensure_bucket(table, index);

    ll_item_t *current = bucket->head;
    while (current) {
        ht_item *existing = (ht_item *)current->data;
        if (table->key_cmp(get_ht_item_key(existing),
                           get_ht_item_key(item)) == 0) {
            /* Key already present – update value using item's value. */
            existing->update_value(existing, get_ht_item_value(item));
            free_ht_item(item); /* Item is redundant now. */
            return;
        }
        current = current->next;
    }

    /* Key not present – append new item. */
    add_to_list(bucket, item);
    table->count++;

    if (load_factor(table) > LOAD_FACTOR_THRESHOLD) {
        ht_resize(table);
    }
}

ht_item *ht_search(const hash_table_t *table, const void *key) {
    if (!table || !key) return NULL;

    int index = table->hash_func(key, table->size);
    linked_list_t *bucket = table->buckets[index];
    if (bucket == NULL) return NULL;

    ll_item_t *current = bucket->head;
    while (current) {
        ht_item *it = (ht_item *)current->data;
        if (table->key_cmp(get_ht_item_key(it), key) == 0) {
            return it;
        }
        current = current->next;
    }
    return NULL;
}

int ht_remove(hash_table_t *table, const void *key,
              void (*free_item)(ht_item *)) {
    if (!table || !key) return 0;

    int index = table->hash_func(key, table->size);
    linked_list_t *bucket = table->buckets[index];
    if (bucket == NULL) return 0;

    ll_item_t *current = bucket->head;
    ll_item_t *previous = NULL;
    while (current) {
        ht_item *it = (ht_item *)current->data;
        if (table->key_cmp(get_ht_item_key(it), key) == 0) {
            /* Unlink current. */
            if (previous == NULL) {
                bucket->head = current->next;
                if (bucket->head == NULL) bucket->tail = NULL;
            } else {
                previous->next = current->next;
                if (current->next == NULL) bucket->tail = previous;
            }
            if (free_item) free_item(it);
            free(current);
            table->count--;
            return 1; /* Removed. */
        }
        previous = current;
        current = current->next;
    }
    return 0; /* Not found. */
}

void free_hash_table(hash_table_t *table, void (*free_item)(ht_item *)) {
    if (!table) return;

    for (int i = 0; i < table->size; ++i) {
        linked_list_t *bucket = table->buckets[i];
        if (bucket == NULL) continue;

        ll_item_t *current = bucket->head;
        while (current) {
            ll_item_t *next = current->next;
            if (free_item) free_item((ht_item *)current->data);
            free(current);
            current = next;
        }
        free(bucket);
    }
    free(table->buckets);
    free(table);
}
int ht_get_count(const hash_table_t *table) {
    return table ? table->count : 0;
}
int ht_get_size(const hash_table_t *table) {
    return table ? table->size : 0;
}



