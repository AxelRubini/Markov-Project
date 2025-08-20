#include "file_writer.h"
#include "frame.h"
#include "next_word_chooser.h"
#include "utf8_tools.h"
#include "utils.h"

#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

/**
 * @brief Check if word is a sentence terminator.
 *
 * @param word Code-point word.
 * @return Non-zero if word is '.', '!' or '?'.
 */
static int is_last_terminator(const int *word) {
  if (word == NULL) {
    fprintf(stderr, "Word is NULL\n");
    return 0; // Not a terminator
  }
  // Check if the word is one of the valid terminators
  return (word[0] == '.' || word[0] == '!' || word[0] == '?');
}

/**
 * @brief Generate a random symbol from the predefined stat
 *
 * @return Pointer to an array of integers representing the symbol.
 */
static int *generate_random_symbol() {
  int *symbol = dmalloc(2 * sizeof(int));
  const int charset[] = GENERATED_TEST_STARTERS;
  int index = rand() % (ARR_SZ(charset));
  symbol[0] = charset[index];
  symbol[1] = '\0';
  return symbol;
}

/**
 * @brief Print all available words in the hash table.
 *
 * @param table Hash table to print words from.
 * @return 0 on success, -1 on error.
 */
static int print_available_words(const hash_table_t *table) {
  if (table == NULL || table->size == 0) {
    fprintf(stderr, "Hash table is NULL or empty\n");
    return -1; // No valid words to print
  }
  for (int i = 0; i < table->size; i++) {
    linked_list_t *bucket = table->buckets[i];
    if (bucket == NULL) {
      continue; // Skip empty buckets
    }
    ll_item_t *current = bucket->head;
    while (current != NULL) {
      ht_item *item = (ht_item *)current->data;
      utf8_print_word(item->key, STDOUT_FILENO);
      printf("\n");
      current = current->next;
    }
  }
  fflush(stdout); // Ensure all output is flushed

  return 0; // Success
}

/**
 * @brief Create a start word from user input or random symbol.
 *
 * @param table Hash table to search for the word.
 * @return Pointer to an array of integers representing the start word or NULL on error.
 */
static int *create_start_word(const hash_table_t *table) {
  if (table == NULL || table->size == 0) {
    fprintf(stderr, "Hash table is NULL or empty\n");
    return NULL; // No valid start word
  }
  fprintf(stdout, "Available words:\n");
  fflush(stdout);
  if (print_available_words(table) < 0) {
    fprintf(stderr, "Failed to print available words\n");
    return NULL; // Error printing words
  }
  fprintf(
      stdout,
      "Enter a starting word from the above list(or press Enter for random): ");
  fflush(stdout);
  // Read a UTF-8 word from stdin
  char *input_word = NULL;
  size_t len = 0;
  ssize_t read = getline(&input_word, &len, stdin);
  if (read < 0) {
    perror("getline");
    free(input_word);
    return NULL; // Error reading input
  }
  // Remove trailing newline character
  if (input_word[read - 1] == '\n') {
    input_word[read - 1] = '\0';
    len--;
  }
  int *word = utf8_rawbytes_to_cp((const u8 *)input_word, len);
  free(input_word); // Free the input buffer
  if (word == NULL) {
    fprintf(stderr, "Failed to convert input to UTF-8 word\n");
    return NULL; // Error converting input
  }
  ht_item *item = ht_search(table, word);
  if (item == NULL) {
    fprintf(stderr, "Word not found in hash table: ");
    utf8_print_word(word, STDOUT_FILENO);
    printf("\n");
    free(word); // Free the word if not found
  find_good_symbol:
    fprintf(stdout, "Generating a random valid start word ...\n");
    int *start_word = generate_random_symbol();
    if (start_word == NULL) {
      fprintf(stderr, "Failed to create start word from random symbol\n");
      return NULL; // Error creating start word
    }
    if ((ht_item *)ht_search(table, start_word) == NULL) {
      // symbol not found, try again
      free(start_word);
      goto find_good_symbol;
    }

    return start_word; // Return the random symbol as the start word
  } else {
    // Word found in hash table, return it
    int *start_word = utf8_str_deepcopy(item->key);
    if (start_word == NULL) {
      fprintf(stderr, "Failed to create start word from input\n");
      free(word); // Free the word if not found
      return NULL;
    }
    free(word);        // Free the word after copying
    return start_word; // Return the found word
  }
}

int write_output_sp(const char *filename, const int num_words,
                    hash_table_t *table) {
  if (filename == NULL || num_words <= 0) {
    fprintf(stderr, "Invalid filename or number of words\n");
    return -1;
  }
  int fd = open(filename, O_WRONLY | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR);
  if (fd < 0) {
    perror("write_output_sp: open file");
    return -1;
  }

  next_word_chooser_t *chooser = create_next_word_chooser(table);
  if (chooser == NULL) {
    fprintf(stderr, "Failed to create next word chooser\n");
    safe_close(&fd);
    return -1;
  }

  int *current_word = create_start_word(table);
  if (current_word == NULL) {
    fprintf(stderr, "Failed to create start word l\n");
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
  safe_close(&fd);
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
    safe_close(&file_fd);
    return -1;
  }

  int words_written = 0;
  while (1) {
    ssize_t frm_st = recv_frame(fd, &buff, &cap);
    if (frm_st < 0) {
      perror("write_output_mp: recv_frame");
      free(buff);
      free(last_word);
      safe_close(&file_fd);
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
      safe_close(&file_fd);
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
  safe_close(&file_fd);
  return 0;
}

int send_output_mp(int fd, hash_table_t *ht, const int num_words) {

  if (fd < 0 || num_words <= 0) {
    fprintf(stderr,
            "Invalid file descriptor, number of words, or start symbol\n");
    return -1;
  }
  next_word_chooser_t *chooser = create_next_word_chooser(ht);
  if (chooser == NULL) {
    fprintf(stderr, "Failed to create next word chooser\n");
    return -1;
  }

  int *current_word = create_start_word(ht);

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
