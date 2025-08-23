#ifndef FILE_WRITER_H
#define FILE_WRITER_H
#include "hash_table.h"

#define GENERATED_TEST_STARTERS {'.', '!', '?'}

/**
 * @brief Generate text and write it to a file (single process).
 *
 * @param filename Output file path.
 * @param num_words Number of words to emit (>0).
 * @param start_sym Starting symbol; updated if NULL.
 * @param table Hash table of word transitions.
 * @return 0 on success, -1 on error.
 */
int write_output_sp(const char *filename, const int num_words,
		    hash_table_t *table);

/**
 * @brief Send generated words over a file descriptor.
 *
 * @param fd Destination descriptor.
 * @param ht Word transition table.
 * @param num_words Number of words to emit.
 * @param start_sym Starting symbol; must not be NULL.
 * @return 0 on success, -1 on error.
 */
int send_output_mp(int fd, hash_table_t *ht, const int num_words);

/**
 * @brief Write frames from a descriptor to a UTF-8 file.
 *
 * @param fd Source descriptor.
 * @param filename Output file path.
 * @return 0 on success, -1 on error.
 */
int write_output_mp(int fd, const char *filename);
#endif // FILE_WRITER_H
