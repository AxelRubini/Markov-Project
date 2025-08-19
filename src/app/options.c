#include "options.h"
#include "argtable3.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static void set_default_options(options_t *o) {
  o->mode = MODE_SP;
  o->task = TASK_BOTH;
  o->input_txt = NULL;
  o->input_csv = NULL;
  o->output_txt = NULL;
  o->output_csv = NULL;
  o->n_words = 0;
}

static char *make_csv_name_from_input(const char *input_txt) {
  const char *prefix = "Analysis_";
  // Extract the base name from the input file path
  const char *base = strrchr(input_txt, '/');
  base = base ? base + 1 : input_txt;

  // Delete the extension
  const char *dot = strrchr(base, '.');
  size_t stem_len = dot ? (size_t)(dot - base) : strlen(base);

  // Compose the output file name
  const char *suffix = ".csv";
  size_t out_len = strlen(prefix) + stem_len + strlen(suffix) + 1;

  char *out = malloc(out_len);
  if (!out)
    return NULL;

  // Build the string
  snprintf(out, out_len, "%s%.*s%s", prefix, (int)stem_len, base, suffix);
  return out;
}

static int is_empty(const char *str) { return str == NULL || str[0] == '\0'; }

static int check_options(options_t *options) {
  // Check if input files are provided for task1 or task both, if not, ask for
  // terminal input
  if (is_empty(options->input_txt) && options->task != TASK_2) {
    fprintf(stderr, "Input file is not provided, please provide an input "
                    "writing on the terminal\n");
    if (is_empty(options->output_csv)) {

      options->output_csv = "Analysis_STDIN_text.csv";
    }
  }
  // Check if intput files is provided for task2, if not return error,if
  // task_both, generate one file to write on.
  if (is_empty(options->input_csv) && options->task == TASK_2) {
    fprintf(
        stderr,
        "Input CSV file is not provided, please provide an input CSV file\n");
    return -1;
  }
  // Check if output files are provided, if not, set them to default values
  if (options->output_txt == NULL && options->task != TASK_1) {
    fprintf(stderr, "Output text file is not provided,\n produced test will be "
                    "printed on STDOUT\n");
  }
  fprintf(stderr, " evaluating output_csv: %d\n",
          is_empty(options->output_csv));
  if (is_empty(options->output_csv) && options->task != TASK_2) {
    fprintf(stderr, "Output CSV file is not provided,\n produced test will be "
                    "in a file called Analysis_<input file>\n");
    options->output_csv =
        make_csv_name_from_input((const char *)options->input_txt);
  }
  // Check if n_words is set, if not, set it to a default value
  if (options->n_words == 0) {
    fprintf(stderr, "Number of words to process is not set, setting to 10\n");
    options->n_words = 10;
  }

  return 0;
}

int parse_options(int argc, char **argv, options_t *out) {
  set_default_options(out);

  /** define the argument table */
  struct arg_lit *sp = arg_lit0(NULL, "sp", "Single Process mode");
  struct arg_lit *mp = arg_lit0(NULL, "mp", "Multi Process mode");

  struct arg_lit *task1 = arg_lit0(NULL, "task1", "Run Task 1");
  struct arg_lit *task2 = arg_lit0(NULL, "task2", "Run Task 2");
  struct arg_lit *task_both = arg_lit0(NULL, "task-both", "Run both tasks");

  struct arg_file *input_txt =
      arg_file0(NULL, "input-txt", "<file>", "Input text file");
  struct arg_file *input_csv =
      arg_file0(NULL, "input-csv", "<file>", "Input CSV file");
  struct arg_file *output_txt =
      arg_file0(NULL, "output-txt", "<file>", "Output text file");
  struct arg_file *output_csv =
      arg_file0(NULL, "output-csv", "<file>", "Output CSV file");

  struct arg_int *n_words =
      arg_int0(NULL, "n-words", "<n>", "Number of words to process");
  struct arg_end *end = arg_end(20);

  void *argtable[] = {sp,         mp,        task1,     task2,
                      task_both,  input_txt, input_csv, output_txt,
                      output_csv, n_words,   end};

  // Parse the command line arguments
  int nerrors = arg_parse(argc, argv, argtable);
  if (nerrors > 0) {
    arg_print_errors(stderr, end, "Error parsing options");
    return -1;
  }

  // Set run mode
  if (sp->count > 0) {
    out->mode = MODE_SP;
  } else if (mp->count > 0) {
    out->mode = MODE_MP;
  }

  // Set task
  if (task1->count > 0) {
    out->task = TASK_1;
  } else if (task2->count > 0) {
    out->task = TASK_2;
  } else if (task_both->count > 0) {
    out->task = TASK_BOTH;
  }

  // Set input and output files
  out->input_txt = input_txt->filename[0];
  out->input_csv = input_csv->filename[0];
  out->output_txt = output_txt->filename[0];
  out->output_csv = output_csv->filename[0];

  // Set number of words and word to process
  out->n_words = n_words->count > 0 ? n_words->ival[0] : 0;

  // Free the argument table
  arg_freetable(argtable, sizeof(argtable) / sizeof(argtable[0]));

  return check_options(out);
}
