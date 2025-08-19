#ifndef CSV_READER_H
#define CSV_READER_H


#include "hash_table.h"

#include <stddef.h>
#include <sys/types.h>

/**
 * @brief Read a line from a CSV descriptor.
 *
 * @param fd CSV file descriptor.
 * @param buf Pointer to a reusable buffer; reallocated if needed.
 * @param cap Pointer to current buffer capacity.
 * @return Bytes read, 0 on EOF, or -1 on error.
 */
ssize_t read_csv_line(int fd, char **buf, size_t *cap);

/**
 * @brief Populate a hash table by reading a CSV file.
 *
 * @param fd CSV file descriptor.
 * @return Populated hash table or NULL on error.
 */
hash_table_t *read_csv_file_sp(const char *filename);

/**
 * @brief Forward a parsed CSV line from one descriptor to another.
 *
 * @param in_fd Source descriptor.
 * @param out_fd Destination descriptor.
 * @return 0 on success, -1 on error.
 */
int publish_line_on_pipe(int in_fd, int out_fd);

/**
 * @brief Build a hash table from CSV lines read on a pipe.
 *
 * @param in_fd Source pipe descriptor.
 * @return Populated hash table or NULL on error.
 */
hash_table_t *read_lines_from_pipe(int in_fd);


#endif // CSV_READER_H
