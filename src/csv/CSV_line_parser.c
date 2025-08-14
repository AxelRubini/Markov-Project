#include "CSV_line_parser.h"
#include "utf8_tools.h"
#include "utils.h"

ssize_t get_key_from_line(const char *line, int *key) {

  if (line == NULL || key == NULL) {
    fprintf(stderr, "Invalid arguments to get_key_from_line\n");
    return -1;
  }

  size_t offset = 0;
  int cp;
  int k_index = 0;
  while ((cp = utf8_getchar_from_buf((const u8 *)line, &offset)) !=
         EOF) {
    if (cp == UTF8_ERROR) {
      fprintf(stderr, "Error reading UTF-8 character from line\n");
      return -1;
    }
    if (cp == ',') { // End of key when we reach a comma
      if (k_index == 0) {
        fprintf(stderr, "Empty key encountered in line\n");
        return -1; // Empty key is not allowed
      }
      key[k_index] = '\0'; // Null-terminate the key
      break;
    }
    key[k_index++] = cp; // Store the codepoint in the key array
  }

  return (ssize_t)offset; // Return the offset to start reading next words
}

linked_list_t *get_next_words_from_line(const char *line, size_t *offset) {
  if (line == NULL || offset == NULL) {
    fprintf(stderr, "Invalid arguments to get_next_words_from_line\n");
    return NULL;
  }

  linked_list_t *next_words = create_linked_list();
  if (next_words == NULL) {
    fprintf(stderr, "Failed to create linked list for next words\n");
    return NULL;
  }

  int cp;
  enum read_nxt_word_st_t state = READING_NXT_WORD;
  int word[MAX_WORD_LENGTH] = {0};
  char probability_str[FLOATING_POINT_CHAR_SPACE] = {0};
  int w_index = 0;
  int p_index = 0;
  double sum_probability = 0.0;

  while (1) {
    switch (state) {
    case READING_NXT_WORD: {
      cp = utf8_getchar_from_buf((const u8 *)line, offset);

      if (cp == EOF) {
        // word not completed, end of file reached
        if (w_index > 0) {
          fprintf(stderr, "Incomplete word at end of line\n");
          free_linked_list(next_words);
          return NULL;
        }
        state = READING_END_OF_FILE;
        break;
      }

      if (cp == UTF8_ERROR) {
        state = READING_NXT_WORD_ERROR;
        break;
      }

      if (cp == ',') {
        // end of word, prepare to read probability
        if (w_index == 0) {
          fprintf(stderr, "Empty word encountered in line\n");
          state = READING_NXT_WORD_ERROR;
          break;
        }
        word[w_index] = '\0';
        state = READING_PROBABILITY;
        break;
      }

      if (cp == '\n' || cp == '\0') {
        // end of line reached without using probability
        if (w_index > 0) {
          fprintf(stderr, "Word without probability at end of line\n");
          free_linked_list(next_words);
          return NULL;
        }
        return next_words;
      }

      // add character to word
      if (w_index >= MAX_WORD_LENGTH - 1) {
        fprintf(stderr, "Word exceeds maximum length in line\n");
        state = READING_NXT_WORD_ERROR;
        break;
      }
      if (cp == ' ' || cp == '\t') {
        // Ignore spaces and tabs
        break;
      }
      word[w_index++] = cp;
      break;
    }

    case READING_PROBABILITY: {
      if (*offset >= strlen(line)) { // Check if we reached the end of the line
        if (p_index > 0) {
          probability_str[p_index] = '\0';
          char *endptr;
          double probability = strtod(probability_str, &endptr);
          sum_probability += probability;
          if (sum_probability > 1.0) {
            fprintf(stderr, "Total probability exceeds 1.0 in line\n");
            state = READING_PROBABILITY_ERROR;
            break;
          }
          if (*endptr != '\0') {
            fprintf(stderr, "Invalid probability format in line\n");
            state = READING_PROBABILITY_ERROR;
            break;
          }

          next_word_t *next_word = create_next_word(word, probability);
          if (next_word == NULL) {
            fprintf(stderr, "Failed to create next word from line\n");
            free_linked_list(next_words);
            return NULL;
          }
          add_to_list(next_words, next_word);
        }
        return next_words;
      }

      u8 ch = line[*offset];
      (*offset)++;

      if (ch == '\n' || ch == '\0' || ch == EOF) {
        // Fine della riga
        if (p_index == 0) {
          fprintf(stderr, "Empty probability encountered in line\n");
          state = READING_PROBABILITY_ERROR;
          break;
        }

        probability_str[p_index] = '\0';
        char *endptr;
        double probability = strtod(probability_str, &endptr);
        sum_probability += probability;
        if (sum_probability > 1.0) {
          fprintf(stderr, "Total probability exceeds 1.0 in line\n");
          state = READING_PROBABILITY_ERROR;
          break;
        }
        if (*endptr != '\0') {
          fprintf(stderr, "Invalid probability format in line\n");
          state = READING_PROBABILITY_ERROR;
          break;
        }

        next_word_t *next_word = create_next_word(word, probability);
        if (next_word == NULL) {
          fprintf(stderr, "Failed to create next word from line\n");
          free_linked_list(next_words);
          return NULL;
        }
        add_to_list(next_words, next_word);
        return next_words;
      }

      if (ch == ',') {
        // Fine della probabilità, crea next_word e continua
        if (p_index == 0) {
          fprintf(stderr, "Empty probability encountered in line\n");
          state = READING_PROBABILITY_ERROR;
          break;
        }

        probability_str[p_index] = '\0';
        char *endptr;
        double probability = strtod(probability_str, &endptr);
        if (*endptr != '\0') {
          fprintf(stderr, "Invalid probability format in line\n");
          state = READING_PROBABILITY_ERROR;
          break;
        }

        next_word_t *next_word = create_next_word(word, probability);
        if (next_word == NULL) {
          fprintf(stderr, "Failed to create next word from line\n");
          free_linked_list(next_words);
          return NULL;
        }
        add_to_list(next_words, next_word);

        // Reset per la prossima coppia parola-probabilità
        w_index = 0;
        p_index = 0;
        memset(word, 0, sizeof(word));
        memset(probability_str, 0, sizeof(probability_str));
        state = READING_NXT_WORD;
        break;
      }

      // Aggiungi carattere alla stringa probabilità
      if (p_index >= FLOATING_POINT_CHAR_SPACE - 1) {
        fprintf(stderr, "Probability string exceeds maximum length in line\n");
        state = READING_PROBABILITY_ERROR;
        break;
      }
      if (ch == ' ' || ch == '\t') {
        break;
      }
      probability_str[p_index++] = ch;
      break;
    }

    case READING_NXT_WORD_ERROR:
      fprintf(stderr, "Error reading next word from line\n");
      free_linked_list(next_words);
      return NULL;

    case READING_PROBABILITY_ERROR:
      fprintf(stderr, "Error reading probability from line\n");
      free_linked_list(next_words);
      if (sum_probability > 1.0) {
        fprintf(stderr, "Total probability exceeds 1.0 in line\n");
      }
      if (w_index > 0) {
        fprintf(stderr, "Incomplete word at end of line\n");
      }
      if (p_index > 0) {
        fprintf(stderr, "Incomplete probability at end of line\n");
      }
      return NULL;

    case READING_END_OF_FILE:
      return next_words;

    default:
      fprintf(stderr, "Unexpected state in get_next_words_from_line\n");
      free_linked_list(next_words);
      return NULL;
    }
  }
}
