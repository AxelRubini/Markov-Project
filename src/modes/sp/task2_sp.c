#include "task2_sp.h"
#include "CSV_reader.h"
#include "file_writer.h"
#include "hash_table.h"

#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

static char generate_random_symbol() {
  const char starter_symbols[] = {'.', '?', '!', '\0'};
  size_t len = strlen(starter_symbols);
  size_t random_index = rand() % (len - 1); // Exclude the null terminator
  return starter_symbols[random_index];
}

static int print_text_on_stdout(const char *filename) {
  if (filename == NULL) {
    fprintf(stderr, "No file from read specified, to write to stdout\n");
    return 1; // Use stdout
  }
  int fd = open(filename, O_RDONLY);
  if (fd < 0) {
    perror("Failed to open output file");
    return -1; // Error opening file
  }
  char buffer[1024];
  ssize_t bytes_read;
  while ((bytes_read = read(fd, buffer, sizeof(buffer))) > 0) {
    if (write(STDOUT_FILENO, buffer, bytes_read) < 0) {
      perror("Failed to write to stdout");
      close(fd);
      return -1;
    }
  }
  if (bytes_read < 0) {
    perror("Failed to read from output file");
    close(fd);
    return -1;
  }
  if (close(fd) < 0) {
    perror("Failed to close output file");
    return -1; 
  }
  return 0; 
}

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

  if (word == NULL) {
    // generate random character from {.,?,!}
    char random_symbol = generate_random_symbol();
    word = &random_symbol;
  } else {
    // find the first valid character to be the starter symbol
    size_t len = strlen(word);
    size_t i = 0;
    while (i < len && word[i] != '.' && word[i] != '?' && word[i] != '!') {
      i++;
    }
    if (i == len) {
      fprintf(stderr, "No valid starter symbol found in the provided word\n");
      fprintf(stderr, "Using a random starter symbol instead\n");
      char random_symbol = generate_random_symbol();
      word = &random_symbol;
    }
    word = &word[i];
  }
  int result = write_output_sp(output_txt, n_words, (char *)word, ht);
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
  close(out_fd);
  free_hash_table(ht);
  if (output_txt != NULL) {
    if (close(out_fd) < 0) {
      perror("Failed to close output file");
      remove(output_txt); // Remove the output file if it was created
      return -1;
    }
  }
  if (print_to_stdout) {
    // Print the content of the output file to stdout
    if (print_text_on_stdout(output_txt) < 0) {
      fprintf(stderr, "Failed to print output to stdout\n");
      remove(output_txt); // Remove the temporary file
      return -1;
    }
    remove(output_txt); // Remove the temporary file after printing
  }
  return 0;
}
