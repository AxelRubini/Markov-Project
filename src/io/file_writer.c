#include "file_writer.h"
#include "frame.h"
#include "next_word_chooser.h"
#include "utils.h"

#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <time.h>
#include <unistd.h>

/**
 * @brief Convert raw UTF-8 bytes to code points.
 *
 * @param bytes Input byte buffer.
 * @param len Number of bytes in buffer.
 * @return Newly allocated array of code points or NULL on error.
 */
int *utf8_rawbytes_to_cp(const u8 *bytes, size_t len) {
  if (!bytes || len == 0) {
    return NULL;
  }

  int *word = malloc(sizeof(int) * MAX_WORD_LENGTH);
  if (!word) {
    return NULL;
  }

  size_t offset = 0;
  int word_index = 0;

  while (offset < len && word_index < MAX_WORD_LENGTH - 1) {
    int cp = utf8_getchar_from_buf(bytes, &offset);
    if (cp == EOF || cp == UTF8_ERROR) {
      break;
    }
    word[word_index++] = cp;
  }

  word[word_index] = '\0';
  return word;
}

/**
 * @brief Read a word from a UTF-8 byte buffer.
 *
 * @param buf Source buffer.
 * @param offset In/out byte offset.
 * @return Newly allocated word or NULL on error.
 */
int *utf8_read_word_from_buf(const u8 *buf, size_t *offset) {
  if (!buf || !offset) {
    fprintf(stderr, "Invalid arguments to utf8_create_word_from_buf\n");
    return NULL;
  }

  int *word = dmalloc(sizeof(int) * MAX_WORD_LENGTH);
  if (!word) {
    fprintf(stderr, "Memory allocation failed for word\n");
    return NULL;
  }

  size_t i = 0;
  while (1) {
    int cp = utf8_getchar_from_buf(buf, offset);
    if (cp == EOF || cp == UTF8_ERROR || cp == '\0') {
      break; // End of buffer or invalid UTF-8 sequence
    }
    if (i >= MAX_WORD_LENGTH - 1) {
      fprintf(stderr, "Word exceeds maximum length\n");
      free(word);
      return NULL; // Exceeded max length
    }
    word[i++] = cp; // Store codepoint in the word
  }
  word[i] = '\0'; // Null-terminate the word

  return word;
}
/**
 * @brief Check if word is a sentence terminator.
 *
 * @param word Code-point word.
 * @return Non-zero if word is '.', '!' or '?'.
 */
int is_last_terminator(const int *word) {
  if (word == NULL) {
    fprintf(stderr, "Word is NULL\n");
    return 0; // Not a terminator
  }
  // Check if the word is one of the valid terminators
  return (word[0] == '.' || word[0] == '!' || word[0] == '?');
}

/**
 * @brief Create a deep-copied start word from a punctuation symbol.
 */
static int *create_start_word(char sym) {
  switch (sym) {
  case '.':
  case '!':
  case '?':
    return utf8_str_deepcopy((int[]){sym, '\0'});
  default:
    fprintf(stderr, "Invalid start symbol: %c\n", sym);
    return NULL;
  }
}
/**
 * @brief Check if the start character is present in the hash table if not will
 *      choose the first start character available in the hash table.
 *      @param start_sym Pointer to the start symbol character.
 *      @param table Pointer to the hash table.
 *      @return 0 if the start symbol is valid, -1 if not.
 */
int *check_start_sym(char *start_sym, hash_table_t *table) {
  if (start_sym == NULL || table == NULL) {
    fprintf(stderr, "Invalid start symbol or hash table\n");
    return NULL;
  }

  // Check if the start symbol is in the hash table
  int *start_word = create_start_word(start_sym[0]);
  if (start_word == NULL) {
    fprintf(stderr, "Failed to create start word from symbol: %c\n",
            *start_sym);
    return NULL;
  }
  if (ht_search(table, start_word) == NULL) {
    // if not found, choose a valid symbol from the hash table
    fprintf(stderr, "Start symbol '%c' not found in the table, searching for "
                    "valid start symbol\n",
            start_sym[0]);
    free(start_word);
    char arr[3] = GENERATED_TEST_STARTERS;
    size_t len = sizeof(arr) / sizeof(arr[0]);
    for (size_t i = 0; i < len; i++) {
      if (arr[i] == start_sym[0]) {
        continue; // Skip the current start symbol
      }
      start_word = create_start_word(arr[i]);
      if (start_word == NULL) {
        fprintf(stderr, "Failed to create start word from symbol: %c\n",
                arr[i]);
        return NULL;
      }
      if (ht_search(table, start_word) != NULL) {
        fprintf(stderr, "Found valid start symbol: %c\n", arr[i]);
        start_sym[0] = arr[i]; // Update the start symbol
        return start_word;     // Return the valid start word
      }
      free(start_word); // Free the previous start word
    }
    fprintf(stderr, "No valid start symbol found in the hash table\n");
    return NULL;
  }
  return start_word;
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
  char sym;
  if (start_sym && *start_sym) {
    sym = *start_sym;
  } else {
    size_t len = sizeof(valid_start_symbols) / sizeof(valid_start_symbols[0]);
    srand(time(NULL));
    sym = valid_start_symbols[rand() % len];
    if (start_sym)
      *start_sym = sym;
  }

  next_word_chooser_t *chooser = create_next_word_chooser(table);
  if (chooser == NULL) {
    fprintf(stderr, "Failed to create next word chooser\n");
    safe_close(&fd);
    return -1;
  }

  int *current_word = check_start_sym(&sym, table);
  if (current_word == NULL) {
    fprintf(stderr, "Failed to create start word from symbol: %c\n", sym);
    next_word_chooser_free(chooser);
    safe_close(&fd);
    return -1;
  }

  int words_written = 0;
  while (words_written < num_words) {
    int *next_word = next_word_chooser_choose(chooser, current_word);
    if (next_word == NULL) {
      fprintf(stderr, "No next word found for the current word\n");
      next_word_chooser_free(chooser);
      free(current_word);
      safe_close(&fd);
      return -1;
    }
    if (is_last_terminator(current_word)) {
      next_word[0] = utf8_char_to_upper(next_word[0]);
    }
    if (!is_last_terminator(next_word)) {
      write(fd, " ", 1); // Write a space after each word
    }
    utf8_print_word(next_word, fd);
    words_written++;
    next_word[0] = utf8_char_to_lower(next_word[0]); // Convert to lowercase
    free((void *)current_word);                      // Free the previous word
    current_word = next_word; // Update current_word to the new word
  }

  free(current_word);
  next_word_chooser_free(chooser);
  if (close(fd) < 0) {
    perror("close");
    return -1;
  }
  return 0; // Success
}

int write_output_mp(int fd, const char *filename) {
  if (fd < 0 || filename == NULL) {
    fprintf(stderr, "Invalid file descriptor or filename\n");
    return -1;
  }

  int file_fd = open(filename, O_WRONLY | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR);
  if (file_fd < 0) {
    perror("write_output_mp: open file");
    return -1;
  }

  char *buff = NULL;
  size_t cap = 0;
  int *last_word =
      utf8_str_deepcopy((int[]){'.', '\0'}); // Start with period as last word
  if (last_word == NULL) {
    fprintf(stderr, "Failed to create initial last word\n");
    close(file_fd);
    return -1;
  }

  int words_written = 0;
  while (1) {
    ssize_t frm_st = recv_frame(fd, &buff, &cap);
    if (frm_st < 0) {
      perror("write_output_mp: recv_frame");
      free(buff);
      free(last_word);
      close(file_fd);
      return -1;
    }
    if (frm_st == 0) {
      break; // End of stream
    }

    // Convert received bytes back to UTF-8 codepoints
    int *current_word = utf8_rawbytes_to_cp((const u8 *)buff, frm_st);
    if (current_word == NULL) {
      fprintf(stderr, "Failed to convert frame to UTF-8 word\n");
      free(buff);
      free(last_word);
      close(file_fd);
      return -1;
    }

    // Apply capitalization rules
    if (is_last_terminator(last_word)) {
      current_word[0] = utf8_char_to_upper(current_word[0]);
    }

    // Write space before word (except for first word and after terminators)
    if (words_written > 0 && !is_last_terminator(current_word)) {
      write(file_fd, " ", 1);
    }

    // Write the word to file
    utf8_print_word(current_word, file_fd);
    words_written++;

    // Convert first letter to lowercase for next iteration
    current_word[0] = utf8_char_to_lower(current_word[0]);

    // Update last_word for next iteration
    free(last_word);
    last_word = current_word; // Transfer ownership
  }

  free(buff);
  free(last_word);
  close(file_fd);
  return 0;
}

int send_output_mp(int fd, hash_table_t *ht, const int num_words,
                   const char *start_sym) {

   if (fd < 0 || num_words <= 0 || !start_sym || !start_sym[0]) {
      fprintf(stderr,
              "Invalid file descriptor, number of words, or start symbol\n");
      return -1;
   }
   next_word_chooser_t *chooser = create_next_word_chooser(ht);
   if (chooser == NULL) {
      fprintf(stderr, "Failed to create next word chooser\n");
      return -1;
   }

   char sym = start_sym[0];
   int *current_word = check_start_sym(&sym, ht);
   if (current_word == NULL) {
      fprintf(stderr, "Failed to create start word from symbol: %c\n",
              start_sym[0]);
      next_word_chooser_free(chooser);
      return -1;
   }

  int words_written = 0;
  char frm_buf[(MAX_WORD_LENGTH - 1) * sizeof(int) + 1] = {
      0}; // Buffer for the frame
  while (words_written < num_words) {
    int *next_word = next_word_chooser_choose(chooser, current_word);
    if (next_word == NULL) {
      fprintf(stderr, "No next word found for the current word\n");
      next_word_chooser_free(chooser);
      return -1;
    }
    size_t frame_size = utf8_cp_to_rawbytes(next_word, (u8 *)frm_buf);
    if (frame_size == 0) {
      fprintf(stderr, "Failed to convert next word to raw bytes\n");
      next_word_chooser_free(chooser);
      return -1;
    }
    if (send_frame(fd, frm_buf, frame_size) < 0) {
      perror("send_output_mp: send_frame");
      next_word_chooser_free(chooser);
      return -1;
    }
    free((void *)current_word); // Free the previous word
    current_word = next_word;   // Update current_word to the new word
    words_written++;
  }
  free((void *)current_word);      // Free the last word
  next_word_chooser_free(chooser); // Free the chooser
  return 0;                        // Success
}
