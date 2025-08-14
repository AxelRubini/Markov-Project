#include "hash_table.h"
#include "utf8_tools.h"
#include "utils.h"
#include "word.h"

#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>


/**
 * @brief Send consecutive word pairs over a pipe.
 *
 * @param fd Input text descriptor.
 * @param wfd Destination pipe descriptor.
 * @return 0 on success, -1 on error.
 */
int publish_word_on_pipe(int fd, int wfd);

/**
 * @brief Rebuild a hash table from word pairs received on a pipe.
 *
 * @param rfd Source pipe descriptor.
 * @param ht Hash table to populate.
 * @return 0 on success, -1 on error.
 */
int read_words_from_pipe(int rfd, hash_table_t *ht);
