#ifndef FILE_READER_H
#define FILE_READER_H

#include "hash_table.h"
#include "ht_item.h"
#include "linked_list.h"
#include "utf8_tools.h"
#include "utils.h"
#include "word.h"

#include <stdio.h>
#include <string.h>

#define HASH_TABLE_SIZE 97 // Initial size of the hash table for words

/**
 * @brief Build a hash table of words read from a file descriptor.
 *
 * @param fd UTF-8 text file descriptor.
 * @return Newly allocated hash table or NULL on error.
 */
hash_table_t *build_word_hashtable_fd(int fd);

/**
 * @brief Read the next word from a descriptor.
 *
 * @param fd UTF-8 text file descriptor.
 * @param word_buffer Buffer for MAX_WORD_LENGTH code points.
 * @retval 0 Success.
 * @retval 1 End of file.
 * @retval -1 Error, sets errno.
 */
int f_get_next_word(int fd, int *word_buffer);

/**
 * @brief Skip to the start of the next word.
 *
 * @param fd UTF-8 text file descriptor.
 */
void move_cursor_to_next_word(int fd);

#endif // FILE_READER_H
