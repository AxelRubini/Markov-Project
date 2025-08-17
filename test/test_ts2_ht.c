#include "hash_table.h"
#include "CSV_reader.h"
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <assert.h>






int main(void) {
  
  const char *input_csv = "test_files/out_test_csv_writer_mp.csv"; 

  hash_table_t *ht = read_csv_file_sp(input_csv);
  if (!ht) {
    fprintf(stderr, "Failed to read CSV file into hash table\n");
    return EXIT_FAILURE;
  }
  

  // Print the hash table to verify its contents

  // Free the hash table
  free_hash_table(ht);

  return EXIT_SUCCESS;

}
