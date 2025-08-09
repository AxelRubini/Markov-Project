#ifndef FILE_READER_H
#define FILE_READER_H

/*
 *The FileReader header is responsible for reading a UTF-8 encoded text file.
 *It provides methods to read from a file, get the word and his next and
 * */

#include "hash_table.h"
#include "ht_item.h"
#include "linked_list.h"
#include "utf8_tools.h"
#include "utils.h"
#include "word.h"

#include <stdio.h>
#include <string.h>

#define HASH_TABLE_SIZE 97 // Initial size of the hash table for words

hash_table_t *build_word_hashtable_fd(int fd);
int f_get_next_word(int fd, int *word_buffer);
static int is_punctuation(const int cp);
void move_cursor_to_next_word(int fd);

#endif // FILE_READER_H
