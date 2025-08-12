#include "../include/file_writer.h"
#include "../include/next_word_chooser.h"

#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>
int is_last_terminator(const int *word) {
  if (word == NULL) {
    fprintf(stderr, "Word is NULL\n");
    return 0; // Not a terminator
  }
  // Check if the word is one of the valid terminators
  return (word[0] == '.' || word[0] == '!' || word[0] == '?');
}
int write_output_sp(const char *filename, const int num_words, char *start_sym,
                    hash_table_t *table) {
  if (filename == NULL || num_words <= 0) {
    fprintf(stderr, "Invalid filename or number of words\n");
    return -1;
  }
  char valid_start_symbols[] = GENERATED_TEST_STARTERS;
  int fd = open(filename, O_WRONLY | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR);
  if (fd < 0) {
    perror("write_output_sp: open file");
    return -1;
  }
  if (start_sym == NULL) {
    int len = sizeof(valid_start_symbols) / sizeof(valid_start_symbols[0]);
    srand(time(NULL)); // Seed the random number generator with current time
    *start_sym = valid_start_symbols[rand() % len];
    return -1;
  }

  next_word_chooser_t *chooser = create_next_word_chooser(table);
  if (chooser == NULL) {
    fprintf(stderr, "Failed to create next word chooser\n");
    close(fd);
    return -1;
  }
  int *start_word = NULL;
  if (*start_sym == '.') {
    start_word = utf8_str_deepcopy((int[]){'.', '\0'}); // Start with a period
  } else if (*start_sym == '!') {
    start_word = utf8_str_deepcopy((int[]){'!', '\0'}); // Start with an exclamation mark
  } else if (*start_sym == '?') {
    start_word = utf8_str_deepcopy((int[]){'?', '\0'}); // Start with a question mark
  } else {
    fprintf(stderr, "Invalid start symbol: %c\n", *start_sym);
    gsl_rng_free(chooser->rng);
    free(chooser);
    close(fd);
    return -1;
  } 
  
  int *current_word =
      start_word; // Initialize current_word with the start symbol;
  if (current_word == NULL) {
    fprintf(stderr, "Failed to create a deep copy of the start symbol\n");
    gsl_rng_free(chooser->rng);
    free(chooser);
    close(fd);
    return -1;
  }
  int words_written = 0;
  while (words_written < num_words) {
    int *next_word = next_word_chooser_choose(chooser, current_word);
    if (next_word == NULL) {
      fprintf(stderr, "No next word found for the current word\n");
      gsl_rng_free(chooser->rng);
      free(chooser);
      close(fd);
      return -1;
    }
    if (is_last_terminator(current_word)) {
      next_word[0] = utf8_char_to_upper(next_word[0]);
    }
    if (is_last_terminator(next_word)) {
      lseek(fd, -2, SEEK_SET);
    }
    utf8_print_word(next_word, fd);
    write(fd, " ", 1); // Write a space after each word
    words_written++;
    next_word[0] = utf8_char_to_lower(next_word[0]); // Convert to lowercase
    free((void *)current_word); // Free the previous word
    current_word = next_word;   // Update current_word to the new word
  }

  return 0; // Success
}
