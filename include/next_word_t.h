#ifndef NEXT_WORD_T_H
#define NEXT_WORD_T_H

#define FREQ_INIT 1 // Initial frequency for a new next word
typedef struct {
  int** next_word;
  int frequency;
} next_word_t;

next_word_t *create_next_word(void *next_word_t);
void update_next_word_frequency(next_word_t *next_word_t);
void free_next_word(next_word_t *next_word_t);
#endif // NEXT_WORD_T_H
