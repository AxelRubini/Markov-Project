#ifndef TASK1_SP_H
#define TASK1_SP_H
#include <stddef.h>

/**
 * @brief Build a word-frequency CSV file from an input text.
 *
 * @param input_txt  Path to the input text file.
 * @param output_csv Path where the resulting CSV will be written.
 * @return 0 on success, negative value on error.
 */
int task1_sp(const char *input_txt, const char *output_csv);
#endif // TASK1_SP_H
