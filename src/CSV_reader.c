#include "../include/CSV_reader.h"
#include "../include/hash_table.h"
#include "../include/ht_data_manipulation.h"
#include "../include/ht_item.h"
#include "../include/linked_list.h"
#include "../include/next_word.h"
#include "../include/utf8_tools.h"
#include "../include/utils.h"
#include "../include/word.h"
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

hash_table_t *csv_to_hashtable(const char *filename) {
  if (!filename) {
    fprintf(stderr, "Invalid filename\n");
    return NULL;
  }

  int fd = open(filename, O_RDONLY);
  if (fd < 0) {
    perror("Failed to open file");
    return NULL;
  }

  hash_table_t *table =
      create_hash_table(HASH_TABLE_SIZE, next_word_hash, keycmp_next_word);
  if (!table) {
    fprintf(stderr, "Failed to create hash table\n");
    close(fd);
    return NULL;
  }
}

int get_key_from_line(int fd, int *buffer) {
  if (fd < 0 || buffer == NULL) {
    fprintf(stderr, "Invalid file descriptor or buffer\n");
    return NULL;
  }
  int bytes_read = 0;
  int cp;
  int index = 0;

  while ((cp = utf8_getchar_fd(fd, &bytes_read)) != EOF || cp != ',') {
    if (cp == UTF8_ERROR) {
      fprintf(stderr, "Error reading character from file\n");
      return -1;
    }
    if (cp == '\n') {
      fprintf(stderr, "Unexpected newline character in key\n");
      return -1;
    }
    if (index >= MAX_WORD_LENGTH - 1) {
      fprintf(stderr, "Key exceeds maximum length\n");
      return -1;
    }
    if (cp == EOF) {
      if (index == 0) {
        fprintf(stderr, "Empty line encountered\n");
        return -1;
      }
      break;
    }
    buffer[index++] = cp;
  }
  buffer[index] = '\0';
  return 0;
}

next_word_t *get_next_word_from_line(int fd, int *end_status) {
  if (fd < 0) {
    fprintf(stderr, "Invalid file descriptor\n");
    *end_status = CSV_ERROR; // Error status
    return NULL;
  }
  int bytes_read = 0;
  int cp;
  int index = 0;
  int pr_index = 0;
  int next_word[MAX_WORD_LENGTH] = {0};
  next_word_t *next_word_obj = dmalloc(sizeof(next_word_t));
  if (!next_word_obj) {
    fprintf(stderr, "Memory allocation failed for next_word_t\n");
    *end_status = CSV_ERROR; // Error status
    return NULL;
  }
  while ((cp = utf8_getchar_fd(fd, &bytes_read)) != EOF) {
    if (cp == UTF8_ERROR) {
      fprintf(stderr, "Error reading character from file\n");
      free(next_word_obj);
      *end_status = CSV_ERROR; // Error status
      return NULL;
    }
    if (cp == ',') {
      break; // End of next word
    }
    if (index >= MAX_WORD_LENGTH - 1) {
      fprintf(stderr, "Next word exceeds maximum length\n");
      free(next_word_obj);
      *end_status = CSV_ERROR; // Error status
      return NULL;
    }
    if (cp == '\n') {
      fprintf(stderr, "Unexpected newline character in next word\n");
      free(next_word_obj);
      *end_status = CSV_END_OF_LINE; // End of line reached
      return NULL;
    }
    if (cp == EOF) {
      fprintf(stderr, "Unexpected end of file while reading next word\n");
      free(next_word_obj);
      *end_status = CSV_END_OF_FILE; // End of file reached
      return NULL;
    }
    next_word[index++] = cp;
  }
  next_word[index] = '\0'; // Null-terminate the next word
  char probability_str[FLOATING_POINT_CHAR_SPACE] = {0};
  if (cp == EOF) {
    fprintf(stderr, "Unexpected end of file while reading probability\n");
    free(next_word_obj);
    *end_status = CSV_END_OF_FILE; // End of file reached
    return NULL;
  }
  // Read the probability value
  int read_res= 0;
  while (1) {
    char prob_char;
    if ((read_res=read(fd, &prob_char, 1)) != 1) {
      fprintf(stderr, "Error reading probability character\n");
      free(next_word_obj);
      *end_status = CSV_ERROR;
      return NULL;
    }
    if (prob_char == '\n') {
      *end_status = CSV_END_OF_LINE; // End of line reached
      break;                         // End of probability value
    }
    if (prob_char == ',') {
      *end_status = CSV_OK;
      break; // Comma indicates end of probability value
    }
    if (read_res == 0) {
      *end_status = CSV_END_OF_FILE; // End of file reached
      break;                         // End of file reached
    }
    if (pr_index >= FLOATING_POINT_CHAR_SPACE - 1) {
      fprintf(stderr, "Probability string exceeds maximum length\n");
      free(next_word_obj);
      *end_status = CSV_ERROR; // Error status
      return NULL;
    }
    probability_str[pr_index++] = prob_char; // Add character to probability string
  }
  probability_str[pr_index] = '\0'; // Null-terminate the probability string
  next_word_obj->word = utf8_str_deepcopy(next_word);
  if (next_word_obj->word == NULL) {
    fprintf(stderr, "Failed to create a deep copy of the next word\n");
    free(next_word_obj);
    *end_status = CSV_ERROR; // Error status
    return NULL;
  }
  // Convert the probability string to a double
  char *endptr;
  double probability = strtod(probability_str, &endptr);
  if (*endptr != '\0') {
    fprintf(stderr, "Invalid probability value: %s\n", probability_str);
    free(next_word_obj->word);
    free(next_word_obj);
    *end_status = CSV_ERROR; // Error status
    return NULL;
  }
  if (probability < 0.0 || probability > 1.0) {
    fprintf(stderr, "Probability value out of range: %f\n", probability);
    free(next_word_obj->word);
    free(next_word_obj);
    *end_status = CSV_ERROR; // Error status
    return NULL;
  } else {
    next_word_obj->probability = probability; // Set the probability value
    *end_status = CSV_OK;                     // Success status

    return next_word_obj;
  }
}
