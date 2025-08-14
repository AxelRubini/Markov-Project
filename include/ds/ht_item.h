#ifndef HT_ITEM_H
#define HT_ITEM_H

typedef struct ht_item ht_item;
typedef void (*update_value_func_t)(const void *item, const void *new_value);
typedef void (*free_item_func_t)(ht_item *item);
typedef void (*print_item_func_t)(const ht_item *item, int fd);
struct ht_item {
  void *key;                        // Pointer to the key
  void *value;                      // Pointer to the value
  update_value_func_t update_value; // Function to update the value
  free_item_func_t free_item;       // Function to free the item
  print_item_func_t print_item;// Function to print the item
};

/**
 * @brief Create a generic hash-table item.
 *
 * @param key Key pointer.
 * @param value Value pointer.
 * @return Newly allocated item.
 */
ht_item *default_create_ht_item(void *key, void *value);

/**
 * @brief Get the key stored in an item.
 *
 * @param item Hash-table item.
 * @return Key pointer.
 */
void *get_ht_item_key(const ht_item *item);

/**
 * @brief Get the value stored in an item.
 *
 * @param item Hash-table item.
 * @return Value pointer.
 */
void *get_ht_item_value(const ht_item *item);

/**
 * @brief Free an item using its free callback.
 *
 * @param item Item to free.
 */
void free_ht_item(ht_item *item);

/**
 * @brief Default deallocator for ht_item.
 *
 * @param item Item to free.
 */
void default_free_ht_item(ht_item *item);
#endif
