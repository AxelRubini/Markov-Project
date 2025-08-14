#include "hash_table.h"
#include "CSV_reader.h"
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <assert.h>






int main(void) {
  
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

  // Print the hash table to verify its contents

  // Free the hash table
  free_hash_table(ht);

  return EXIT_SUCCESS;

}
