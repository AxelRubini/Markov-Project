#ifndef HT_ITEM_H
#define HT_ITEM_H

typedef struct {
  void *key;   // Pointer to the key
  void *value; // Pointer to the value
  void (*update_value)(void *item, void *new_value); // Function pointer to update the value
} ht_item;


ht_item *create_ht_item(void *key, void *value, void (*update_value)(void *item, void *new_value));
void *get_ht_item_key(const ht_item *item);
void *get_ht_item_value(const ht_item *item);
void free_ht_item(ht_item *item);
#endif
