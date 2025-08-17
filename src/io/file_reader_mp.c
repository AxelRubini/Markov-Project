#include "file_reader_mp.h"
#include "frame.h"

/**
 * @brief Skip the next bytes if typographic character.
 *
 * @param fd File descriptor to read from.
 * @return 0 on success, -1 on error.
 */
static int check_next_typograph(int fd) {
  if (fd < 0) {
    fprintf(stderr, "Invalid file descriptor\n");
    return -1;
  }
  int bytes_read = 0;
  int cp = utf8_getchar_fd(fd, &bytes_read);
  if (is_typographic(cp)) {

    return -1;
  }
  lseek(fd, -bytes_read, SEEK_CUR); // Rewind to allow next read
  return 0;                         // Successfully skipped
}

/**
 * @brief Read and return the next non-space character.
 *
 * @param fd Source file descriptor.
 * @return Character found or EOF on error/end.
 */
char move_to_next_word(int fd) {
  if (fd < 0) {
    fprintf(stderr, "Invalid file descriptor\n");
    return -1;
  }
  char buf[1] = {0};
  int rd_st;
  check_next_typograph(fd); // Check if next is typographic

  while ((rd_st = read(fd, buf, 1)) > 0) {
    // Read until finding a character that is not space or newline
    if (buf[0] == ' ' || buf[0] == '\n' || is_punctuation(buf[0])) {
      check_next_typograph(fd); // Check if next is typographic
      continue;
    }
    // If we find a valid character, return it
    return buf[0];
  }
  return EOF; // EOF or read error
}

/**
 * @brief Create a "current next" frame string.
 *
 * @param cw_data Current word bytes.
 * @param cw_len Length of current word.
 * @param nxw_data Next word bytes.
 * @param nxw_len Length of next word.
 * @return Newly allocated frame or NULL on error.
 */
char *create_string_for_frame(const char *cw_data, size_t cw_len,
                              const char *nxw_data, size_t nxw_len) {
  if (cw_len == 0 || nxw_len == 0) {
    fprintf(stderr, "Invalid word lengths\n");
    return NULL;
  }
  size_t total_len = cw_len + nxw_len + 2; // space + null terminator
  char *frame = malloc(total_len);
  if (!frame) {
    fprintf(stderr, "Memory allocation failed\n");
    return NULL;
  }
  memcpy(frame, cw_data, cw_len);
  frame[cw_len] = ' ';
  memcpy(frame + cw_len + 1, nxw_data, nxw_len);
  frame[total_len - 1] = '\0';
  return frame;
}

/**
 * @brief Read the next word from a descriptor.
 *
 * @param fd Source descriptor.
 * @param nxt_buf Buffer for the word bytes.
 * @param len Output length.
 * @retval 0 Success.
 * @retval 1 End of file.
 * @retval -1 Error.
 */
int read_next_word(int fd, char *nxt_buf, size_t *len) {
  if (fd < 0 || !nxt_buf || !len) {
    fprintf(stderr, "Invalid file descriptor or buffer\n");
    return -1;
  }
  size_t b_index = 0;
  *len = 0;
  char c = move_to_next_word(fd);
  if (c == EOF)
    return 1;

  if (is_sentence_terminator(c)) {
    nxt_buf[0] = c;
    nxt_buf[1] = '\0';
    *len = 1;
    check_next_typograph(fd); // Check if next is typographic
    return 0;
  }

  nxt_buf[b_index++] = c;
  (*len)++;

  while (read(fd, &c, 1) > 0) {
    if (is_sentence_terminator(c)) {
      lseek(fd, -1, SEEK_CUR);
      break;
    }
    if (c == ' ' || c == '\n')
      break;
    if (is_punctuation(c))
      continue;
    if (b_index >= MAX_WORD_LENGTH - 2) {
      fprintf(stderr, "Word exceeds maximum length\n");
      return -1;
    }
    if (c == '\'') {
      // Keep apostrophe in the word
      nxt_buf[b_index++] = c;
      (*len)++;
      break;
    }
    nxt_buf[b_index++] = c;
    (*len)++;
    check_next_typograph(fd); // Check if next is typographic
  }

  nxt_buf[b_index] = '\0';
  return 0;
}

int publish_word_on_pipe(int fd, int wfd) {
  if (fd < 0 || wfd < 0) {
    fprintf(stderr, "Invalid file descriptors to publish_word_on_pipe\n");
    return -1;
  }

  size_t cw_ln = 0;
  size_t nxw_ln = 0;
  size_t first_word_ln = 0;
  char current_word[MAX_WORD_LENGTH_BYTES] = {0};
  char next_word[MAX_WORD_LENGTH_BYTES] = {0};
  char first_word[MAX_WORD_LENGTH_BYTES] = {0}; // Store the first word

  // Read first word
  int rd_st = read_next_word(fd, current_word, &cw_ln);
  if (rd_st != 0) {
    // No words to read or error
    return (rd_st == 1) ? 0 : -1; // Return success if EOF, error otherwise
  }

  // Save the first word for circular pairing
  memcpy(first_word, current_word, cw_ln);
  first_word[cw_ln] = '\0';
  first_word_ln = cw_ln;

  // Process pairs of words
  while ((rd_st = read_next_word(fd, next_word, &nxw_ln)) == 0) {
    if (cw_ln == 0 || nxw_ln == 0) {
      fprintf(stderr, "Empty word encountered\n");
      // Move next word to current and continue
      memset(current_word, 0, sizeof(current_word));
      if (nxw_ln > 0) {
        memcpy(current_word, next_word, nxw_ln);
        current_word[nxw_ln] = '\0';
        cw_ln = nxw_ln;
      }
      memset(next_word, 0, sizeof(next_word));
      nxw_ln = 0;
      continue;
    }

    char *frame =
        create_string_for_frame(current_word, cw_ln, next_word, nxw_ln);
    if (!frame) {
      fprintf(stderr, "Failed to create frame for words\n");
      return -1; // Error creating frame
    }

    size_t frame_len = strlen(frame);
    if (send_frame(wfd, frame, frame_len) < 0) {
      fprintf(stderr, "Failed to send frame on pipe\n");
      free(frame);
      return -1; // Error sending frame
    }

    free(frame); // Free frame after sending

    // Prepare for next iteration
    memset(current_word, 0, sizeof(current_word));
    memcpy(current_word, next_word, nxw_ln);
    current_word[nxw_ln] = '\0';
    cw_ln = nxw_ln;

    memset(next_word, 0, sizeof(next_word));
    nxw_ln = 0;
  }

  if (rd_st < 0) {
    fprintf(stderr, "Error reading next word\n");
    return -1; // Error reading word
  }

  // rd_st == 1 means EOF reached
  // Now create final frame: last_word + first_word
  if (cw_ln > 0 && first_word_ln > 0) {
    char *final_frame =
        create_string_for_frame(current_word, cw_ln, first_word, first_word_ln);
    if (!final_frame) {
      fprintf(stderr, "Failed to create final frame\n");
      return -1;
    }

    size_t final_frame_len = strlen(final_frame);
    if (send_frame(wfd, final_frame, final_frame_len) < 0) {
      fprintf(stderr, "Failed to send final frame on pipe\n");
      free(final_frame);
      return -1;
    }

    free(final_frame);
  }

  return 0; // Success
}

int read_words_from_pipe(int rfd, hash_table_t *ht) {
  if (rfd < 0 || ht == NULL) {
    fprintf(stderr, "Invalid file descriptor or hash table\n");
    return -1;
  }

  char *line = NULL;
  size_t cap = 0;
  int key[MAX_WORD_LENGTH] = {0};      // Buffer for the key
  int nxt_word[MAX_WORD_LENGTH] = {0}; // Buffer for next words
  int cp;

  while (1) {
    ssize_t n = recv_frame(rfd, &line, &cap);
    if (n < 0) {
      perror("recv_frame");
      free(line);
      return -1; // Error receiving frame
    }

    if (n == 0) {
      break; // EOF reached
    }

    if (line[0] == '\0') {
      continue; // Skip empty lines
    }

    // Reset variables for each frame
    int key_index = 0;
    int nxt_word_index = 0;
    size_t off = 0; // Reset offset for each new line

    // Clear buffers
    memset(key, 0, sizeof(key));
    memset(nxt_word, 0, sizeof(nxt_word));

    // Parse the first word (key)
    while (off < (size_t)n && line[off] != ' ' && line[off] != '\0') {
      if (key_index >= MAX_WORD_LENGTH - 1) {
        fprintf(stderr, "Key exceeds maximum length\n");
        free(line);
        return -1; // Error: key too long
      }

      cp = utf8_getchar_from_buf((u8 *)line, &off);
      if (cp == UTF8_ERROR) {
        fprintf(stderr, "Error reading UTF-8 character from line\n");
        free(line);
        return -1; // Error reading character
      }
      if (cp == EOF) {
        fprintf(stderr, "Unexpected EOF while reading key\n");
        free(line);
        return -1; // Error: unexpected EOF
      }
      if (cp == '\0') {
        break; // End of string
      }
      if (cp == ' ') {
        break; // End of key - space found
      }
      key[key_index++] = cp; // Store character in key buffer
    }

    if (key_index == 0) {
      fprintf(stderr, "Empty key found in line\n");
      free(line);
      return -1; // Error: empty key
    }
    key[key_index] = '\0'; // Null-terminate the key

    // Skip the space character
    if (off < (size_t)n && line[off] == ' ') {
      off++;
    } else {
      fprintf(stderr, "Expected space between key and next word\n");
      free(line);
      return -1;
    }

    // Parse the second word (next word)
    while (off < (size_t)n && line[off] != '\0') {
      if (nxt_word_index >= MAX_WORD_LENGTH - 1) {
        fprintf(stderr, "Next word exceeds maximum length\n");
        free(line);
        return -1; // Error: next word too long
      }

      cp = utf8_getchar_from_buf((u8 *)line, &off);
      if (cp == UTF8_ERROR) {
        fprintf(stderr, "Error reading UTF-8 character from line\n");
        free(line);
        return -1; // Error reading character
      }
      if (cp == EOF) {
        break; // End of input
      }
      if (cp == '\0') {
        break; // End of string
      }
      nxt_word[nxt_word_index++] = cp; // Store character in next word buffer
    }

    // Check if next word is empty
    if (nxt_word_index == 0) {
      fprintf(stderr, "Empty next word found in line\n");
      free(line);
      return -1; // Error: empty next word
    }
    nxt_word[nxt_word_index] = '\0'; // Null-terminate the next word

    // Create word objects
    word_t *key_word = create_word(key);
    if (!key_word) {
      fprintf(stderr, "Failed to create word from key buffer\n");
      free(line);
      return -1; // Error creating word
    }

    word_t *value_word = create_word(nxt_word);
    if (!value_word) {
      fprintf(stderr, "Failed to create word from next word buffer\n");
      // Note: Need to free key_word properly - this depends on implementation
      // Assuming create_word allocates memory that needs to be freed
      free(key_word);
      free(line);
      return -1; // Error creating word
    }

    ht_item *item = word_ht_item_create(key_word, value_word);
    if (!item) {
      fprintf(stderr, "Failed to create hash table item\n");
      // Free the word objects - implementation dependent
      free(key_word);
      free(value_word);
      free(line);
      return -1; // Error creating hash table item
    }

    ht_insert(ht, item); // Insert item into hash table
  }

  free(line); // Free the line buffer
  return 0;   // Success
}
