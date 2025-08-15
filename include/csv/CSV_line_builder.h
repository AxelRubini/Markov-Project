#ifndef CSV_LINE_BUILDER_H
#define CSV_LINE_BUILDER_H
#include "hash_table.h"
#include "ht_item.h"
#include "linked_list.h"
#include "utf8_tools.h"
#include "utils.h"
#include "word.h"
#include <stddef.h>

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
size_t calculate_buffer_size(ht_item *item);

#endif // CSV_LINE_BUILDER_H
