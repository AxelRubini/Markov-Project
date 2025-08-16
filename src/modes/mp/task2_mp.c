#include "task2_mp.h"
#include "CSV_reader.h"
#include "file_writer.h"
#include "hash_table.h"
#include "modes_utils.h"

#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

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

static int w_p1(int pipe_to_write, const char *input_csv) {
  if (pipe_to_write < 0) {
    fprintf(stderr, "Invalid pipe descriptor for writing\n");
    return -1;
  }
  int fd = open(input_csv, O_RDONLY);
  if (fd < 0) {
    perror("Failed to open input CSV file");
    return -1; // Error opening file
  }
  int result = publish_line_on_pipe(fd, pipe_to_write);
  if (result < 0) {
    fprintf(stderr, "Error reading CSV file\n");
    close(fd);
    return -1; // Error reading CSV file}

    if (close(fd) < 0) {
      perror("Failed to close input CSV file");
      return -1; // Error closing file
    }
  }
  if (close(pipe_to_write) < 0) {
    perror("Failed to close pipe after writing");
    return -1; // Error closing pipe
  }
  return 0; // Success
}

static int w_p2(int pipe_to_read, int pipe_to_write, size_t n_words,
                const char *word) {
  if (pipe_to_read < 0 || pipe_to_write < 0) {
    fprintf(stderr, "Invalid pipe descriptors for reading or writing\n");
    return -1;
  }
  hash_table_t *ht = read_lines_from_pipe(pipe_to_read);
  if (ht == NULL) {
    fprintf(stderr, "Error reading lines from pipe\n");
    return -1; // Error reading from pipe
  }
  if (close(pipe_to_read) < 0) {
    perror("Failed to close pipe after reading");
    free_hash_table(ht);
    return -1; // Error closing pipe
  }
  int result = send_output_mp(pipe_to_write, ht, n_words, word);
  if (result < 0) {
    fprintf(stderr, "Error writing lines to pipe\n");
    free_hash_table(ht);
    return -1; // Error writing to pipe
  }
  if (close(pipe_to_write) < 0) {
    perror("Failed to close pipe after writing");
    free_hash_table(ht);
    return -1; // Error closing pipe
  }
  free_hash_table(ht);
  return 0; // Success
}

static int w_p3(int pipe_to_read, const char *output_txt) {
  if (pipe_to_read < 0) {
    fprintf(stderr, "Invalid pipe descriptor for reading\n");
    return -1;
  }
  int fd = open(output_txt, O_WRONLY | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR);
  if (fd < 0) {
    perror("Failed to open output file for writing");
    return -1; // Error opening file
  }
  int result = write_output_mp(pipe_to_read, output_txt);
  if (result < 0) {
    fprintf(stderr, "Error reading from pipe and writing to file\n");
    close(fd);
    return -1; // Error reading from pipe
  }
  if (close(fd) < 0) {
    perror("Failed to close output file");
    return -1; // Error closing file
  }
  return 0; // Print to stdout if needed
}
int task2_mp(const char *input_csv, const char *output_txt, size_t n_words,
             const char *word) {

  int print_to_stdout = 0;
  int out_fd = -1;
  if (output_txt == NULL) {
    fprintf(stderr, "No output file specified, writing to stdout\n");
    print_to_stdout = 1;
    output_txt = "tmp_output.txt"; // Temporary file for stdout
  }

  char *validated_word = validate_word(word);

 
  int pipe12[2], pipe23[2];
  if (pipe(pipe12) != 0) {
    perror("pipe12");
    return 1;
  }
  if (pipe(pipe23) != 0) {
    perror("pipe23");
    close(pipe12[0]); // Clean up pipe12 in case of error
    close(pipe12[1]);
    return 1;
  }
  pid_t p1 = fork();
  if (p1 < 0) {
    perror("fork p1");

    close(pipe12[0]);
    close(pipe12[1]);
    close(pipe23[0]);
    close(pipe23[1]);
    return 1;
  }
  if (p1 == 0) {
    close(pipe12[0]); 
    close(pipe23[0]); 
    close(pipe23[1]);
    int rc = w_p1(pipe12[1], input_csv);
    if (rc != 0) {
      fprintf(stderr, "p1 failed with error code %d\n", rc);
      close(pipe12[1]);
      _exit(rc);
    }
    _exit(0); 
  }
  pid_t p2 = fork();
  if (p2 < 0) {
    perror("fork p2");
    
    kill(p1, SIGTERM);
    close(pipe12[0]);
    close(pipe12[1]);
    close(pipe23[0]);
    close(pipe23[1]);
    return 1;
  }
  if (p2 == 0) {
    close(pipe12[1]); 
    close(pipe23[0]); 
    int rc = w_p2(pipe12[0], pipe23[1], n_words, validated_word);
    if (rc != 0) {
      fprintf(stderr, "p2 failed with error code %d\n", rc);
      close(pipe12[0]);
      close(pipe23[1]);
      _exit(rc);
    }
    _exit(0); 
  }
  pid_t p3 = fork();
  if (p3 < 0) {
    perror("fork p3");
    
    kill(p1, SIGTERM);
    kill(p2, SIGTERM);
    close(pipe12[0]);
    close(pipe12[1]);
    close(pipe23[0]);
    close(pipe23[1]);
    return 1;
  }
  if (p3 == 0) {
    
    close(pipe12[0]); 
    close(pipe12[1]);
    close(pipe23[1]); 
    int rc = w_p3(pipe23[0], output_txt);
    if (rc != 0) {
      fprintf(stderr, "p3 failed with error code %d\n", rc);
      close(pipe23[0]);
      _exit(rc);
    }
    _exit(0); 
  }
  
  close(pipe12[0]);
  close(pipe12[1]);
  close(pipe23[0]);
  close(pipe23[1]);
  int st1 = 0, st2 = 0, st3 = 0;
  waitpid(p1, &st1, 0);
  waitpid(p2, &st2, 0);
  waitpid(p3, &st3, 0);
  int rc = 0;
  if (WIFEXITED(st1)) {
    if (WEXITSTATUS(st1) != 0) {
      fprintf(stderr, "p1 exited with status %d\n", WEXITSTATUS(st1));
      rc = 1;
    }
  } else {
    fprintf(stderr, "p1 terminated by signal %d\n", WTERMSIG(st1));
    rc = 1;
  }
  if (WIFEXITED(st2)) {
    if (WEXITSTATUS(st2) != 0) {
      fprintf(stderr, "p2 exited with status %d\n", WEXITSTATUS(st2));
      rc = 1;
    }
  } else {
    fprintf(stderr, "p2 terminated by signal %d\n", WTERMSIG(st2));
    rc = 1;
  }
  if (WIFEXITED(st3)) {
    if (WEXITSTATUS(st3) != 0) {
      fprintf(stderr, "p3 exited with status %d\n", WEXITSTATUS(st3));
      rc = 1;
    }
  } else {
    fprintf(stderr, "p3 terminated by signal %d\n", WTERMSIG(st3));
    rc = 1;
  }
  if (rc != 0) {
    remove(output_txt); 
    return rc; 
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
