#ifndef HT_DATA_MANIPULATION_H
#define HT_DATA_MANIPULATION_H
#include "hash_table.h"
#include "ht_item.h"
#include "linked_list.h"
#include "utils.h"
#include "utf8_tools.h"
#include "word.h"




void create_csv_line(ht_item *item, char *buffer);
int calculate_buffer_size(ht_item *item);








#endif // HT_DATA_MANIPULATION_H
