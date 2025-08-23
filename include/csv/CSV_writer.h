#ifndef CSV_WRITER_H
#define CSV_WRITER_H

#include "hash_table.h"

/**
 * @brief Serialize a hash table into a CSV file.
 *
 * @param filename Output file path.
 * @param table Hash table to serialize.
 * @return 0 on success, -1 on error.
 */
int csv_build_file(const char *filename, hash_table_t *table);

/**
 * @brief Send CSV lines for all table entries over a descriptor.
 *
 * @param fd Destination descriptor.
 * @param ht Hash table to serialize.
 * @return 0 on success, -1 on error.
 */
int csv_line_send(int fd, const hash_table_t *ht);

/**
 * @brief Receive CSV frames and write them to a file.
 *
 * @param fd Source descriptor.
 * @param filename Output file path.
 * @return 0 on success, -1 on error.
 */
int csv_line_recv(int fd, const char *filename);

#endif // CSV_WRITER_H
