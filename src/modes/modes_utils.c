#include "modes_utils.h"
#include "utils.h"

#include <signal.h>
#include <sys/wait.h>
static int read_from_stdin(const char *output_txt) {
  fprintf(stderr, "Reading from stdin \n"
                  "if you not redirect stdin, write on the console and press"
                  "Ctrl+D to finish input\n");
  int fd = open(output_txt, O_WRONLY | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR);
  if (fd < 0) {
    perror("Failed to open output file");
    return -1;
  }

  char buffer[1024];
  ssize_t bytes_read;
  while ((bytes_read = read(STDIN_FILENO, buffer, sizeof(buffer))) > 0) {
    if (write(fd, buffer, bytes_read) < 0) {
      perror("Failed to write to temporary file");
      safe_close(&fd);
      return -1;
    }
  }
  safe_close(&fd);
  return 0;
}

const char *validate_input_file(const char *input_txt, int *is_stdin) {
  if (is_empty(input_txt)) {
    if (read_from_stdin(TMP_STDIN_FILE) != 0) {
      fprintf(stderr, "Failed to read from stdin\n");
      return NULL; // Error reading from stdin
    }
    *is_stdin = 1;                    // Indicate that input is from stdin
    return TMP_STDIN_FILE; // Return temporary file name
  } else {
    *is_stdin = 0; // Indicate that input is from a file
    int fd = open(input_txt, O_RDONLY);
    if (fd < 0) {
      perror("Failed to open input file");
      return NULL; // Error opening file
    }
    safe_close(&fd);
    return input_txt; // Return the original file name
  }
}

int print_file_content_to_stdout(const char *filename) {
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
      safe_close(&fd);
      return -1;
    }
  }
  if (bytes_read < 0) {
    perror("Failed to read from output file");
    safe_close(&fd);
    return -1;
  }
  safe_close(&fd);
  return 0;
}

void kill_and_wait(pid_t pid) {
  if (pid > 0) {
    kill(pid, SIGTERM);    // Send termination signal to the process
    waitpid(pid, NULL, 0); // Wait for the process to terminate
  }
}

int is_empty(const char *str) { return str == NULL || *str == '\0'; }
