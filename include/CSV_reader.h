#ifndef CSV_READER_H
#define CSV_READER_H

/*
 * The CSV_reader is responsible for reading a CSV file and populating a hash
 * table with the data. It provides methods to read a CSV file, parse its
 * contents, and store the data in a hash table.
 */

#include "hash_table.h"
#include "ht_item.h"
#include "linked_list.h"
#include "next_word.h"
#include "utf8_tools.h"
#include "utils.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

enum {
  CSV_OK = 0,         // Maximum length of a word
  CSV_END_OF_LINE = 1, // Maximum size of the buffer for reading lines
  CSV_ERROR = -1,     // Error reading the CSV file
  CSV_END_OF_FILE = 2 // End of file reached
}csv_status_t;

hash_table_t *csv_to_hashtable(const char *filename);
ht_item *parse_csv_line(int fd);
int get_key_from_line(int fd, int *buffer);
next_word_t *get_next_word_from_line(int fd, int *end_status);

#endif // CSV_READER_H
