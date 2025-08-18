#ifndef OPTIONS_H
#define OPTIONS_H

#include <stddef.h> 

/**
 * @brief Execution mode of the application.
 */
typedef enum {
  MODE_SP, /**< Run in single-process mode. */
  MODE_MP  /**< Run in multi-process mode. */
} run_mode_t;

/**
 * @brief Available tasks that can be executed.
 */
typedef enum {
  TASK_1,    /**< Execute only Task 1. */
  TASK_2,    /**< Execute only Task 2. */
  TASK_BOTH  /**< Execute both tasks sequentially. */
} task_t;

/**
 * @brief Aggregates command line options for the program.
 *
 * Holds the run mode, task selection, file paths and other parameters
 * supplied by the user.
 */
typedef struct {
  run_mode_t mode;       /**< Selected run mode. */
  task_t task;           /**< Task to execute. */
  const char *input_txt; /**< Path to input text file. */
  const char *input_csv; /**< Path to input CSV file. */
  const char *output_txt; /**< Path to output text file, or NULL for STDOUT. */
  const char *output_csv; /**< Path to output CSV file. */
  size_t n_words;        /**< Number of words to process. */
  const char *word;      /**< Starting word for generation, or NULL for random. */

} options_t;


/**
 * @brief Parse command line arguments into an options structure.
 *
 * @param argc Number of command line arguments.
 * @param argv Array of argument strings.
 * @param out  Structure to fill with parsed options.
 * @return 0 on success, -1 on error.
 *
 * @retval -1 If the arguments are invalid or memory allocation fails.
 */
int parse_options(int argc, char **argv, options_t *out);


#endif // OPTIONS_H
