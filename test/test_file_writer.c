#include "../include/CSV_reader.h"
#include "../include/file_writer.h"

#include "../include/hash_table.h"
#include <assert.h>
#include <fcntl.h>
#include <gsl/gsl_randist.h>
#include <gsl/gsl_rng.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

int main(void) {

  const char *filename = "generated_output.txt";
  int num_words = 100;
  char start_sym = '.';
  int fd = open("test_files/out_test_reader_utf8.csv", O_RDONLY);
  if (fd < 0) {
    perror("Failed to open test file");
    return EXIT_FAILURE;
  }
  hash_table_t *ht = read_csv_file_sp(fd);
  if (!ht) {
    fprintf(stderr, "Failed to read CSV file into hash table\n");
    close(fd);
    return EXIT_FAILURE;
  }
  close(fd);
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
