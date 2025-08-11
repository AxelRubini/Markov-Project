#include "../include/CSV_reader.h"
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>

/*
 * Read a CSV line (ended with \n or EOF) from the file descriptor fd.
 * *buf will be allocated or reallocated using cap if necessary.
 * returns: bytes readed, or -1 on error, or 0 if end of file reached.
 * NOTE: The double dereference of buf is used to allow the function to modify
 * the pointer of the calling function.
 */

ssize_t read_csv_line(int fd, char **buf, size_t *cap) {
  if (!buf || !cap || fd < 0) {
    fprintf(stderr, "Invalid arguments to read_csv_line\n");
    return -1;
  }
  if (*buf == NULL || *cap == 0) {
    *cap = 1024; // Initial capacity
    *buf = (char *)malloc(*cap);
    if (*buf == NULL) {
      fprintf(stderr, "Memory allocation failed for buffer\n");
      return -1;
    }
  }
  size_t len = 0;
  while (1) {
    char ch;
    ssize_t read_res = read(fd, &ch, 1);
    if (read_res == 0) {
      if (len == 0) {
        break; // End of file reached
      }
    }
    if (read_res < 0) {
      if (errno == EINTR) {
        continue; // Interrupted, try again
      }
      fprintf(stderr, "Error reading from file descriptor %d\n", fd);
      free(*buf);
      *buf = NULL;
      return -1; // Error reading
    }
    if (len + 1 >= *cap) {
      size_t new_cap = (*cap < 4096)
                           ? (*cap * 2)
                           : (*cap + 4096); // increase capacity if greater than
                                            // 4096 than plus 4096 not double
      char *new_buf = (char *)realloc(*buf, new_cap);
      if (new_buf == NULL) {
        fprintf(stderr, "Memory reallocation failed for buffer\n");
        free(*buf);
        *buf = NULL;
        return -1; // Error reallocating
      }
      *buf = new_buf;
      *cap = new_cap;
    }
    (*buf)[len++] = ch;
    if (ch == '\n') {
      break; // End of line reached
    }
  }

  (*buf)[len] = '\0';  // Null-terminate the string
  return (ssize_t)len; // Return the number of bytes read
}

int publish_line_on_pipe(int in_fd, int out_fd) {
  char *line = NULL;
  size_t cap = 0;

  while (1) {
    ssize_t n = read_csv_line(in_fd, &line, &cap);

    if (n < 0) {
      perror("read_line_fd");
      free(line);
      return -1;
    }

    if (n == 0) {
      break; // Tutte le righe processate
    }

    // Send the line as frame

    if (send_frame(out_fd, line, (size_t)n) < 0) {
      perror("send_frame");
      free(line);
      return -1;
    }
  }

  free(line);
  return 0;
}
