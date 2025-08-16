#include "task1_sp.h"
#include "CSV_line_builder.h"
#include "CSV_writer.h"
#include "file_reader.h"
#include "utils.h"
#include <fcntl.h>
#include <stdio.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

int task1_sp(const char *input_txt, const char *output_csv) {

  if (input_txt == NULL) {

    int fd = open("tmp.txt", O_WRONLY | O_CREAT | O_TRUNC, 0644);

    if (fd < 0) {
      perror("Failed to open temporary file");
      return -1;
    }

    // Scan standard input and write to the temporary file
    char buffer[1024];
    ssize_t bytes_read;
    while ((bytes_read = read(STDIN_FILENO, buffer, sizeof(buffer))) > 0) {
      if (write(fd, buffer, bytes_read) < 0) {
        perror("Failed to write to temporary file");
        close(fd);
        return -1;
      }
    }
    close(fd);
  }
  int fd = open(input_txt, O_RDONLY);
  if (fd < 0) {
    perror("Failed to open input file");
    return -1;
  }

  int in_fd = open(input_txt ? input_txt : "tmp.txt", O_RDONLY);
  if (in_fd < 0) {
    perror("Failed to open input file");
    close(fd);
    return -1;
  }
  hash_table_t *ht = build_word_hashtable_fd(in_fd);
  close(in_fd);
  if (ht == NULL) {
    fprintf(stderr, "Failed to build hash table from input file\n");
    close(fd);
    return -1;
  }
  csv_build_file(output_csv, ht);
  free_hash_table(ht);
  remove("tmp.txt"); // Remove the temporary file if it was created
  close(fd);
  return 0;
}
