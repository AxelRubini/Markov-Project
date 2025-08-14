#ifndef OPTIONS_H
#define OPTIONS_H

#include <stddef.h> 

typedef enum { MODE_SP, MODE_MP } run_mode_t;
typedef enum { TASK_1, TASK_2, TASK_BOTH } task_t;

typedef struct {
  run_mode_t mode;
  task_t task;
  const char *input_txt;
  const char *input_csv;
  const char *output_txt;
  const char *output_csv;
  size_t n_words;
  const char *word;

} options_t;


int parse_options(int argc, char **argv, options_t *out);


#endif // OPTIONS_H
