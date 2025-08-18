#include "task2_sp.h"
#include "CSV_reader.h"
#include "file_writer.h"
#include "hash_table.h"
#include "modes_utils.h"

#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

int task2_sp(const char *input_csv, const char *output_txt, size_t n_words,
             const char *word) {
  int print_to_stdout = 0;
  int out_fd = -1;
  if (output_txt == NULL) {
    fprintf(stderr, "No output file specified, writing to stdout\n");
    print_to_stdout = 1;
    output_txt = "tmp_output.txt"; // Temporary file for stdout
  }

  hash_table_t *ht = read_csv_file_sp(input_csv);
  if (ht == NULL) {
    fprintf(stderr, "Failed to read CSV file into hash table\n");
    return -1;
  }
  if (n_words == 0) {
    fprintf(stderr, "Number of words to process cannot be zero\n");
    free_hash_table(ht);
    return -1;
  }

  char *validated_word = validate_word(word);

  int result = write_output_sp(output_txt, n_words, validated_word, ht);
  if (validated_word && validated_word != word) {
    free(validated_word);
  }
  if (result < 0) {
    fprintf(stderr, "Failed to send output: %d\n", result);
    free_hash_table(ht);
    if (output_txt != NULL) {
      remove(output_txt); // Remove the output file if it was created
    }
    free_hash_table(ht);
    return result;
  }
  fprintf(stderr, "Output written to %s\n", output_txt);
  free_hash_table(ht);
 
  if (print_to_stdout) {
    // Print the content of the output file to stdout
    if (print_file_content_to_stdout(output_txt) < 0) {
      fprintf(stderr, "Failed to print output to stdout\n");
      remove(output_txt); // Remove the temporary file
      return -1;
    }
    unlink(output_txt); // Remove the temporary file after printing
  }
  return 0;
}
