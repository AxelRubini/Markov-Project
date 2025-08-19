#include "file_reader.h"
#include "utf8_tools.h"
#include "word.h"
#include "utils.h"

#include <stdio.h>
#include <string.h>
#include <unistd.h>

hash_table_t *build_word_hashtable_fd(int fd) {
  if (fd < 0) {
    fprintf(stderr, "Invalid file descriptor\n");
    return NULL;
  }

  hash_table_t *table =
      create_hash_table(HASH_TABLE_SIZE, word_hash, word_hashtable_keycmp);
  if (table == NULL) {
    fprintf(stderr, "Failed to create hash table\n");
    return NULL;
  }

  int current_word[MAX_WORD_LENGTH] = {0}; // buffer for the current word
  int next_word[MAX_WORD_LENGTH] = {0};    // buffer for the next word

  // buffer for first word
  int first_word[MAX_WORD_LENGTH] = {0};
  // read initial word
  move_cursor_to_next_word(fd);
  int status = f_get_next_word(fd, current_word);

  if (status == 1) {
    // empty file
    return table;
  }
  if (status == -1) {
    // read error
    free_hash_table(table);
    return NULL;
  }

  memcpy(first_word, current_word, sizeof(current_word));

  // main loop to read word pairs
  while (1) {
    // ensure current_word is valid
    if (current_word[0] == 0) {
      // empty word, attempt to read next
      move_cursor_to_next_word(fd);
      status = f_get_next_word(fd, current_word);
      if (status != 0)
        break;
      continue;
    }

    // create word_t for current_word
    word_t *word = create_word(current_word);
    if (word == NULL) {
      fprintf(stderr, "Failed to create word from current_word\n");
      // skip and try next word
      move_cursor_to_next_word(fd);
      status = f_get_next_word(fd, current_word);
      if (status != 0)
        break;
      continue;
    }

    // read the following word
    move_cursor_to_next_word(fd);
    int next_status = f_get_next_word(fd, next_word);

    if (next_status == 1) {
      word_t *first_word_obj = create_word(first_word);
      if (first_word_obj) {
        ht_item *item = word_ht_item_create(word, first_word_obj);
        if (item) {
          ht_insert(table, item);
          free_word(first_word_obj);
          free_word(word);
        } else {
          fprintf(stderr, "Failed to create hash table item for first word\n");
          free_word(first_word_obj);
          free_word(word);
        }
      } else {
        fprintf(stderr, "Failed to create first word from first_word buffer\n");
        free_word(word);
      }
      break;
    }

    if (next_status == -1) {
      // error reading following word
      free_word(word);
      break;
    }

    // ensure next_word is valid
    if (next_word[0] == 0) {
      // empty next word, discard current and continue
      free_word(word);
      // move current_word forward for next iteration
      memset(current_word, 0, sizeof(current_word));
      move_cursor_to_next_word(fd);
      status = f_get_next_word(fd, current_word);
      if (status != 0)
        break;
      continue;
    }

    // create word_t for next_word
    word_t *next_word_obj = create_word(next_word);
    if (next_word_obj == NULL) {
      fprintf(stderr, "Failed to create next word from next_word\n");
      free_word(word);
      // copy next_word into current_word for next iteration
      memcpy(current_word, next_word, sizeof(current_word));
      continue;
    }

    // create and insert item into hash table
    ht_item *item = word_ht_item_create(word, next_word_obj);
    if (item == NULL) {
      fprintf(stderr, "Failed to create hash table item\n");
      free_word(word);
      free_word(next_word_obj);
      // copy next_word into current_word for next iteration
      memcpy(current_word, next_word, sizeof(current_word));
      continue;
    }

    ht_insert(table, item);
    free_word(word);
    free_word(next_word_obj);

    // prepare for next iteration: next_word becomes current_word
    memcpy(current_word, next_word, sizeof(current_word));
    memset(next_word, 0, sizeof(next_word));
  }

  return table;
}
int f_get_next_word(int fd, int *word_buffer) {
  if (fd < 0) {
    fprintf(stderr, "Invalid file descriptor\n");
    return -1;
  }

  int bytes_read = 0;
  int index = 0;
  int cp;

  // skip spaces and ignorable punctuation
  while ((cp = utf8_getchar_fd(fd, &bytes_read)) != EOF) {
    if (cp == UTF8_ERROR) {
      fprintf(stderr, "Error reading UTF-8 character\n");
      return -1;
    }

    // skip spaces or newline
    if (cp == ' ' || cp == '\n') {
      continue;
    }

    // skip punctuation
    if (is_punctuation(cp)) {
      continue;
    }

    // sentence terminators become standalone words
    if (cp == '.' || cp == '?' || cp == '!') {
      word_buffer[index++] = cp;
      word_buffer[index] = 0;
      return 0;
    }

    // first valid character of a word
    word_buffer[index++] = cp;
    break;
  }

  // EOF without any character
  if (cp == EOF) {
    if (index == 0) {
      return 1; // EOF
    }
  }

  // read rest of the word
  while ((cp = utf8_getchar_fd(fd, &bytes_read)) != EOF) {
    if (cp == UTF8_ERROR) {
      fprintf(stderr, "Error reading UTF-8 character\n");
      return -1;
    }

    // stop on space or newline
    if (cp == ' ' || cp == '\n') {
      break;
    }

    // stop on sentence terminator and rewind
    if (cp == '.' || cp == '?' || cp == '!') {
      lseek(fd, -bytes_read, SEEK_CUR);
      break;
    }

    // stop at apostrophe
    if (cp == '\'') {
      word_buffer[index++] = cp; // keep apostrophe
      word_buffer[index] = 0;    // null-terminate
      return 0;                  // word complete
    }

    // skip ignorable punctuation
    if (is_punctuation(cp)) {
      continue;
    }

    // enforce maximum length
    if (index >= MAX_WORD_LENGTH - 1) {
      fprintf(stderr, "Word exceeds maximum length\n");
      return -1;
    }

    // append character
    word_buffer[index++] = cp;
  }

  word_buffer[index] = 0; // null-terminate
  return 0;
}

void move_cursor_to_next_word(int fd) {
  int cp;
  int bytes_read = 0;

  while ((cp = utf8_getchar_fd(fd, &bytes_read)) != EOF) {
    // on UTF-8 error advance one byte and continue
    if (cp == UTF8_ERROR) {
      lseek(fd, 1, SEEK_CUR);
      continue;
    }

    // skip spaces or newline
    if (cp == ' ' || cp == '\n') {
      continue;
    }

    // skip punctuation
    if (is_punctuation(cp)) {
      continue;
    }

    // rewind to let f_get_next_word read first valid character
    lseek(fd, -bytes_read, SEEK_CUR);
    return;
  }
}
