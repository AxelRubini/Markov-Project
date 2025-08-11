#ifndef CSV_LINE_PARSER_H
#define CSV_LINE_PARSER_H

#include "ht_item.h"
#include "linked_list.h"
#include "next_word.h"
#include "utf8_tools.h"
#include "utils.h"
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>


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
 * The CSV_line_parser is responsible for parsing a single line from a CSV file
 * and creating a hash table item from it. It provides methods to create a CSV
 * line from a hash table item and to read a CSV line from a file descriptor.
 */

/*
 *  This function extracts the key from a CSV line.
 *  because is the first word in the line.
 *  @param line The CSV line to parse.
 *  @param key Pointer to an integer where the key will be stored.
 *  @return the offset to offset to start reading the next words
 *  or -1 if an error occurs.
 */
size_t get_key_from_line(const char *line, int *key);
/*
 * This function extracts the next words from a CSV line.
 * It starts reading from the given offset and returns a linked list of next words.
 * @param line The CSV line to parse.
 * @param offset Pointer to an integer that will be updated with the new offset.
 * @return A linked list containing the next words, or NULL if an error occurs.
 */
linked_list_t *get_next_words_from_line(const char *line, size_t *offset);

#endif // CSV_LINE_PARSER_H
