#ifndef MODES_UTILS_H
#define MODES_UTILS_H

#include <sys/types.h>

/**
 * @brief Temporary file used to store data read from stdin when no input file
 * is provided.
 */
#define TMP_STDIN_FILE "tmp_read_from_stdin.txt"

/**
 * @brief Temporary file used to capture output when no output file is provided.
 */
#define TMP_STDOUT_FILE "tmp_output.txt"

/**
 * @brief Validate or read input file.
 *
 * When @p input_txt is NULL the function reads from standard input and
 * stores the content in a temporary file.
 *
 * @param input_txt Path to the input file or NULL to read from stdin.
 * @param is_stdin  Output flag set to 1 if reading from stdin, 0 otherwise.
 * @return Pointer to the validated file path on success, NULL on error.
 */
const char *validate_input_file(const char *input_txt, int *is_stdin);

/**
 * @brief Print file content to stdout.
 * @param filename File to read and print.
 * @return 0 on success, -1 on error.
 */
int print_file_content_to_stdout(const char *filename);

/**
 * @brief Validate a word or generate a random symbol.
 *
 * If @p word is NULL or invalid, a random punctuation symbol is returned.
 *
 * @param word Word or symbol to validate.
 * @return Newly allocated valid word or symbol, or NULL on failure.
 *         Caller is responsible for freeing the returned string.
 */
char *validate_word(const char *word);

/**
 * @brief Terminate a process and wait for its completion.
 *
 * @param pid Process identifier to terminate.
 */
void kill_and_wait(pid_t pid);

int is_empty(const char *str);

#endif // MODES_UTILS_H
