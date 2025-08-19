#ifndef CSV_LINE_PARSER_H
#define CSV_LINE_PARSER_H

#include "linked_list.h"

#include <stddef.h>
#include <sys/types.h>


enum read_nxt_word_st_t{
  READING_NXT_WORD,
  READING_NXT_WORD_ERROR,
  READING_PROBABILITY,
  READING_PROBABILITY_ERROR,
  READING_END_OF_LINE,
  READING_END_OF_LINE_ERROR,
  READING_END_OF_FILE,
  READING_END_OF_FILE_ERROR

};

/**
 * @brief Extract the key from a CSV line.
 *
 * @param line NUL-terminated CSV line.
 * @param key Output buffer for the key as code points.
 * @return Offset after the key or -1 on error.
 */
ssize_t get_key_from_line(const char *line, int *key);

/**
 * @brief Parse next words and probabilities from a CSV line.
 *
 * @param line NUL-terminated CSV line.
 * @param offset In/out byte offset to continue parsing.
 * @return Linked list of next_word_t or NULL on error.
 */
linked_list_t *get_next_words_from_line(const char *line, size_t *offset);

#endif // CSV_LINE_PARSER_H
