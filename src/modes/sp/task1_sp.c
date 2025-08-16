#include "task1_sp.h"
#include "CSV_writer.h"
#include "file_reader.h"
#include "modes_utils.h"
#include <fcntl.h>
#include <stdio.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

int task1_sp(const char *input_txt, const char *output_csv) {
  int is_stdin = 0;
  char *validated_input_txt = validate_input_file(input_txt, &is_stdin);
  if (validated_input_txt == NULL) {
    fprintf(stderr, "Invalid input file\n");
    return -1;
  }

  

  int in_fd = open(validated_input_txt, O_RDONLY);
  if (in_fd < 0) {
    perror("Failed to open input file");
    return -1;
  }

 
  hash_table_t *ht = build_word_hashtable_fd(in_fd);
  close(in_fd);
  if (ht == NULL) {
    fprintf(stderr, "Failed to build hash table from input file\n");
    close(in_fd);
    return -1;
  }
  csv_build_file(output_csv, ht);
  free_hash_table(ht);
  remove("tmp.txt"); // Remove the temporary file if it was created
  close(in_fd);
  return 0;
}
