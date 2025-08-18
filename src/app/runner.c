#include "runner.h"
#include "options.h"

#include "both_mp.h"
#include "both_sp.h"
#include "task1_mp.h"
#include "task1_sp.h"
#include "task2_mp.h"
#include "task2_sp.h"

#include <stdio.h>

int run(const options_t *options) {

  int res;

  if (options->mode == MODE_SP) {
    printf("Running in Single Process mode...\n");
    if (options->task == TASK_1) {
      printf("Executing Task 1...\n");
      res = task1_sp(options->input_txt, options->output_csv);
    } else if (options->task == TASK_2) {
      printf("Executing Task 2...\n");
      res = task2_sp(options->input_csv, options->output_txt, options->n_words,
                     options->word);
    } else {
      printf("Executing both tasks...\n");
      res = both_sp(options->input_txt, options->output_csv,
                    options->output_txt, options->n_words, options->word);
    }

  } else {
    printf("Running in Multi Process mode...\n");
    if (options->task == TASK_1) {
      printf("Executing Task 1...\n");
      res = task1_mp(options->input_txt, options->output_csv);
    } else if (options->task == TASK_2) {
      printf("Executing Task 2...\n");
      res = task2_mp(options->input_csv, options->output_txt, options->n_words,
                     options->word);
    } else {
      printf("Executing both tasks...\n");
      res = both_mp(options->input_txt, options->output_csv,
                    options->output_txt, options->n_words, options->word);
    }
  }

  return res;

}
