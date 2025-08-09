#ifndef HT_ITEM_H
#define HT_ITEM_H

typedef struct ht_item ht_item;
typedef void (*update_value_func_t)(const void *item, const void *new_value);
typedef void (*free_item_func_t)(ht_item *item);
struct ht_item {
  void *key;                        // Pointer to the key
  void *value;                      // Pointer to the value
  update_value_func_t update_value; // Function to update the value
  free_item_func_t free_item;       // Function to free the item
};

ht_item *default_create_ht_item(void *key, void *value);
void *get_ht_item_key(const ht_item *item);
void *get_ht_item_value(const ht_item *item);
void free_ht_item(ht_item *item);
void default_free_ht_item(ht_item *item); 
#endif
