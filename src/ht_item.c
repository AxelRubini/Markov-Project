#include "../include/ht_item.h"
#include "../include/utils.h"
#include <stdio.h>
#include <stdlib.h>

void default_update_value(const void *item, const void *new_value) {
  if (item == NULL || new_value == NULL) {
    fprintf(stderr, "Item or new value is NULL\n");
    return;
  }
  ht_item *htItem = (ht_item *)item;
  htItem->value = new_value; // Update the value directly
}

ht_item *default_create_ht_item(void *key, void *value) {
  ht_item *new_item = dmalloc(sizeof(ht_item));
  new_item->key = key;
  new_item->value = value; // Initialize value to NULL}
  new_item->update_value = default_update_value;
  new_item->free_item = default_free_ht_item; // No free function by default
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

void default_free_ht_item(ht_item *item) {
  if (item == NULL) {
    fprintf(stderr, "ht_item is NULL\n");
    return;
  }
  free(item);
}
