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
#include <sys/types.h>

/**
 * @Reads a line from the CSV file and populates the hash table with the data.
 *
 * @param fd The file descriptor of the CSV file.
 * @param table The hash table to populate with the data.
 * @return csv_status_t indicating the status of the operation.
 */
ssize_t read_csv_line(int fd, char **buf, size_t *cap);
int publish_line_on_pipe(int in_fd, int out_fd);

#endif // CSV_READER_H
