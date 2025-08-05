#include "../include/ht_item.h"
#include <stdio.h>
#include <stdlib.h>

ht_item *create_ht_item(void *key, void *value) {
  ht_item *new_item = dmalloc(sizeof(ht_item));
  new_item->key = key;
  new_item->value = value;      // Initialize value to NULL}
  return new_item;
}



void *get_ht_item_key(const ht_item *item) {
  if (item == NULL) {
    fprintf(stderr, "ht_item is NULL\n");
    return NULL;
  }
  return item->key;
}

void *get_ht_item_value(const ht_item *item) {
  if (item == NULL) {
    fprintf(stderr, "ht_item is NULL\n");
    return NULL;
  }
  return item->value;
}

void free_ht_item(ht_item *item) {
  if (item == NULL) {
    fprintf(stderr, "ht_item is NULL\n");
    return;
  }
  free(item);
}

