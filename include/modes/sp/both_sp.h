#ifndef BOTH_SP_H
#define BOTH_SP_H
#include <stddef.h>

/**
 * @brief Execute both single-process tasks: build a CSV from text and generate
 *        output text from the CSV.
 *
 * @param input_txt   Path to the input text file.
 * @param output_csv  Path where the generated CSV file will be written.
 * @param output_txt  Path for the generated text, or NULL to use stdout.
 * @param n_words     Number of words to generate in the output.
 * @param word        Optional starting word for text generation.
 * @return 0 on success, negative value on error.
 */
int both_sp(const char *input_txt, const char *output_csv,
            const char *output_txt, size_t n_words, const char *word);

#endif // BOTH_SP_H
