#ifndef BOTH_MP_H
#define BOTH_MP_H

#include <stddef.h>
#include <stdint.h>

/**
 * @brief Execute both tasks in multi-process mode.
 *
 * Builds a frequency table from the input text and generates output text
 * using that table.
 *
 * @param input_txt Path to UTF-8 input text file. NULL uses stdin.
 * @param output_csv Destination CSV file for the frequency table.
 * @param output_txt Destination file for generated text. NULL prints to
 *                   stdout.
 * @param n_words Number of words to generate.
 * @param word Starting word for generation. NULL selects a random start.
 * @return 0 on success, -1 on error.
 */
int both_mp(const char *input_txt, const char *output_csv,
	    const char *output_txt, uint32_t n_words);
#endif // BOTH_MP_H
