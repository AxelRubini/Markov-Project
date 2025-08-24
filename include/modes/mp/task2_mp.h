#ifndef TASK2_MP_H
#define TASK2_MP_H

#include <stddef.h>
#include <stdint.h>

/**
 * @brief Generate text from a CSV frequency table using multiple processes.
 *
 * @param input_csv Path to CSV file containing the frequency table.
 * @param output_txt Destination file for generated text. NULL prints to
 *                   stdout.
 * @param n_words Number of words to generate.
 * @param word Starting word for generation. NULL selects a random start.
 * @return 0 on success, -1 on error.
 */
int task2_mp(const char *input_csv, const char *output_txt, uint32_t n_words);

#endif // TASK2_MP_H
