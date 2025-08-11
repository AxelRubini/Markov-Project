#ifndef CSV_WRITER_H
#define CSV_WRITER_H

/*
 * The CSVwriter header is responsible for writing data to a CSV file.
 * it provides methods to open a file preprocess the strings and write it to a
 * file.
 */

#include "ht_data_manipulation.h"
#include "utf8_tools.h"
#include "utils.h"
#include <stdio.h>
#include <stdlib.h>


int csv_build_file(const char *filename, hash_table_t *table);

#endif // CSV_WRITER_H
