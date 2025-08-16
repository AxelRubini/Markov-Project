#include "runner.h"
#include "options.h"

#include "both_mp.h"
#include "task1_mp.h"
#include "task2_mp.h"
#include "task1_sp.h"
#include "task2_sp.h"
#include "both_sp.h"

#include <stdio.h>






int run(const options_t *options) {

  // Print the options for debugging
  printf("Running with options:\n");
  printf("Mode: %s\n", options->mode == MODE_SP ? "Single Process" : "Multi Process");
  printf("Task: %s\n", options->task == TASK_1 ? "Task 1" : 
                     (options->task == TASK_2 ? "Task 2" : "Both Tasks"));
  printf("Input Text File: %s\n", options->input_txt ? options->input_txt : "None");
  printf("Input CSV File: %s\n", options->input_csv ? options->input_csv : "None");
  printf("Output Text File: %s\n", options->output_txt ? options->output_txt : "None");
  printf("Output CSV File: %s\n", options->output_csv ? options->output_csv : "None");
  printf("Number of Words: %zu\n", options->n_words);
  printf("Word to Process: %s\n", options->word ? options->word : "None");

  if (options->mode == MODE_SP) {
    printf("Running in Single Process mode...\n");
    if (options->task == TASK_1) {
      printf("Executing Task 1...\n");
      task1_sp(options->input_txt, options->output_csv);
    } else if (options->task == TASK_2) {
      printf("Executing Task 2...\n");
      task2_sp(options->input_csv, options->output_txt, options->n_words, options->word);
    } else {
      printf("Executing both tasks...\n");
      both_sp(options->input_txt, options->output_csv, 
              options->output_txt, options->n_words, options->word);      
    }

  }else {
    printf("Running in Multi Process mode...\n");
    if (options->task == TASK_1) {
      printf("Executing Task 1...\n");
      task1_mp(options->input_txt, options->output_csv);
    } else if (options->task == TASK_2) {
      printf("Executing Task 2...\n");
      task2_mp(options->input_csv, options->output_txt, options->n_words, options->word);
    } else {
      printf("Executing both tasks...\n");
      both_mp(options->input_txt, options->output_csv, 
              options->output_txt, options->n_words, options->word);
    }
  }

  return 0;
}
