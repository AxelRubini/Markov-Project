#ifndef HT_DATA_MANIPULATION_H
#define HT_DATA_MANIPULATION_H
#include "hash_table.h"
#include "ht_item.h"
#include "linked_list.h"
#include "utils.h"
#include "utf8_tools.h"
#include "word.h"




/**
 * @brief Build a CSV line from a hash-table item.
 *
 * @param item Item holding a word and its followers.
 * @param buffer Destination buffer.
 * @return 0 on success, -1 on error.
 */
int create_csv_line(ht_item *item, char *buffer);

/**
 * @brief Compute the buffer size required for an item's CSV line.
 *
 * @param item Hash-table item to serialize.
 * @return Required byte count.
 */
int calculate_buffer_size(ht_item *item);








#endif // HT_DATA_MANIPULATION_H
