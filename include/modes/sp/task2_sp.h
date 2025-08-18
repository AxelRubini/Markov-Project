#ifndef TASK2_SP_H
#define TASK2_SP_H
#include <stddef.h>

/**
 * @brief Generate text using a CSV dictionary produced by task1.
 *
 * @param input_csv  Path to the input CSV file.
 * @param output_txt Path for the generated text, or NULL to write to stdout.
 * @param n_words    Number of words to generate.
 * @param word       Optional starting word for generation.
 * @return 0 on success, negative value on error.
 */
int task2_sp(const char *input_csv, const char *output_txt, size_t n_words, const char *word);

#endif // TASK2_SP_H
