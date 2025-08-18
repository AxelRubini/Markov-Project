#ifndef TASK1_MP_H
#define TASK1_MP_H

#include <stddef.h>

/**
 * @brief Build a word frequency table from a text file using multiple
 * processes.
 *
 * @param input_txt Path to UTF-8 input text file. NULL uses stdin.
 * @param output_csv Destination CSV file for the frequency table.
 * @return 0 on success, -1 on error.
 */
int task1_mp(const char *input_txt, const char *output_csv);

#endif // TASK1_MP_H
