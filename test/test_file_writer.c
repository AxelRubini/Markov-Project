#include "CSV_reader.h"
#include "file_writer.h"

#include "hash_table.h"
#include <assert.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

int main(void) {

  const char *filename = "generated_output.txt";
  int num_words = 100;
  char start_sym = '.';
  const char *input_csv = "test_files/out_test_csv_writer_mp.csv";
  hash_table_t *ht = read_csv_file_sp(input_csv);
  if (!ht) {
    fprintf(stderr, "Failed to read CSV file into hash table\n");
    return EXIT_FAILURE;
  }
  print_hash_table(ht,1);
  int result = write_output_sp(filename, num_words, &start_sym, ht);
  if (result < 0) {
    fprintf(stderr, "Failed to write output to file\n");
    free_hash_table(ht);
    return EXIT_FAILURE;
  }
  printf("Output written to %s successfully\n", filename);
  free_hash_table(ht);
  return EXIT_SUCCESS;
}
