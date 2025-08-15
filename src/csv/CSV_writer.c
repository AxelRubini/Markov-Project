#include "CSV_writer.h"

#include "CSV_line_builder.h"
#include "frame.h"
#include "utils.h"
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

int csv_build_file(const char *filename, hash_table_t *table) {
  if (!filename || !table) {
    fprintf(stderr, "Invalid filename or hash table\n");
    return -1;
  }

  FILE *file = fopen(filename, "w");
  if (!file) {
    fprintf(stderr, "Failed to open file %s for writing\n", filename);
    return -1;
  }
  char *buffer = dmalloc(1024);
  if (!buffer) {
    fprintf(stderr, "Memory allocation failed for buffer\n");
    fclose(file);
    return -1;
  }
  ll_item_t *current = NULL;
  for (int i = 0; i < table->size; ++i) {
    linked_list_t *bucket = table->buckets[i];
    if (bucket == NULL)
      continue;
    current = bucket->head;
    while (current) {
      ht_item *item = (ht_item *)current->data;
      size_t buffer_size = calculate_buffer_size(item) * 2;
      if (buffer_size == 0) {
        fprintf(stderr, "Failed to calculate buffer size for item\n");
        free(buffer);
        fclose(file);
        return -1;
      }
      char *new_buffer = realloc(buffer, buffer_size);
      if (!new_buffer) {
        fprintf(stderr, "Memory allocation failed for buffer\n");
        free(buffer);
        fclose(file);
        return -1;
      }
      buffer = new_buffer;
      memset(buffer, 0, buffer_size); // Clear the buffer before use
      create_csv_line(item, buffer);
      size_t line_length = strlen(buffer);
      fwrite(buffer, sizeof(char), line_length, file);
      if (ferror(file)) {
        fprintf(stderr, "Error writing to file %s\n", filename);
        free(buffer);
        fclose(file);
        return -1;
      }
      current = current->next;
    }
  }
  free(buffer);
  if (fclose(file) != 0) {
    fprintf(stderr, "Failed to close file %s\n", filename);
    return -1;
  }
  printf("CSV file %s created successfully\n", filename);

  return 0;
}

int csv_line_send(int fd, const hash_table_t *ht) {
  if (fd < 0 || !ht) {
    fprintf(stderr, "Invalid file descriptor or hash table\n");
    return -1;
  }

  if (!ht->buckets) {
    fprintf(stderr, "Hash table has no buckets\n");
    return -1;
  }

  ll_item_t *current = NULL;
  size_t items_sent = 0;

  for (int i = 0; i < ht->size; ++i) {
    linked_list_t *bucket = ht->buckets[i];
    if (bucket == NULL) {
      continue;
    }

    current = bucket->head;
    while (current) {
      if (!current->data) {
        fprintf(stderr, "Found NULL data in bucket %d\n", i);
        current = current->next;
        continue;
      }

      ht_item *item = (ht_item *)current->data;

      // Calculate buffer size
      size_t buf_sz = calculate_buffer_size(item);
      if (buf_sz == 0) {
        fprintf(stderr, "Invalid buffer size (%zu) for item in bucket %d\n",
                buf_sz, i);
        current = current->next;
        continue;
      }

      // Allocate buffer
      char *buffer = (char *)dmalloc(buf_sz);
      if (!buffer) {
        fprintf(stderr, "Memory allocation failed for buffer of size %zu\n",
                buf_sz);
        return -1;
      }

      // Initialize buffer
      memset(buffer, 0, buf_sz);

      // Create CSV line
      int csv_result = create_csv_line(item, buffer);
      if (csv_result < 0) {
        fprintf(stderr, "Failed to create CSV line for item in bucket %d\n", i);
        free(buffer);
        current = current->next;
        continue;
      }

      // Get actual line length
      size_t line_length = strlen(buffer);
      if (line_length == 0) {
        fprintf(stderr, "Empty CSV line created for item in bucket %d\n", i);
        free(buffer);
        current = current->next;
        continue;
      }

      // Ensure line ends with newline if not already present
      if (line_length > 0 && buffer[line_length - 1] != '\n') {
        if (line_length < buf_sz - 1) {
          buffer[line_length] = '\n';
          buffer[line_length + 1] = '\0';
          line_length++;
        } else {
          fprintf(stderr, "Buffer too small to add newline\n");
          free(buffer);
          return -1;
        }
      }

      // Send frame
      if (send_frame(fd, buffer, line_length) < 0) {
        fprintf(stderr, "Failed to send frame for item in bucket %d\n", i);
        free(buffer);
        return -1;
      }

      free(buffer);
      items_sent++;
      current = current->next;
    }
  }

  fprintf(stdout, "Successfully sent %zu CSV lines\n", items_sent);
  return 0;
}

int csv_line_recv(int fd, const char *filename) {
  if (fd < 0 || !filename) {
    fprintf(stderr, "Invalid file descriptor or filename\n");
    return -1;
  }

  // Validate filename
  if (strlen(filename) == 0) {
    fprintf(stderr, "Empty filename provided\n");
    return -1;
  }

  FILE *file = fopen(filename, "w");
  if (!file) {
    perror("Failed to open file for writing");
    return -1;
  }

  char *line = NULL;
  size_t cap = 0;
  size_t lines_received = 0;

  while (1) {
    ssize_t n = recv_frame(fd, &line, &cap);

    if (n < 0) {
      perror("recv_frame failed");
      if (line) {
        free(line);
      }
      fclose(file);
      return -1;
    }

    if (n == 0) {
      break; // EOF reached
    }

    // Skip completely empty frames
    if (line == NULL || line[0] == '\0') {
      continue;
    }

    // Write the received data to file
    size_t bytes_to_write = (size_t)n;
    size_t bytes_written = fwrite(line, sizeof(char), bytes_to_write, file);

    if (bytes_written < bytes_to_write) {
      if (ferror(file)) {
        fprintf(stderr, "Error writing to file %s after %zu lines\n", filename,
                lines_received);
      } else {
        fprintf(stderr, "Partial write to file %s: wrote %zu of %zu bytes\n",
                filename, bytes_written, bytes_to_write);
      }
      if (line) {
        free(line);
      }
      fclose(file);
      return -1;
    }

    // Flush to ensure data is written
    if (fflush(file) != 0) {
      fprintf(stderr, "Failed to flush file %s\n", filename);
      if (line) {
        free(line);
      }
      fclose(file);
      return -1;
    }

    lines_received++;

    // Note: Don't free line here as recv_frame manages the buffer reallocation
    // The line buffer will be reused/reallocated by recv_frame in next
    // iteration
  }

  // Clean up
  if (line) {
    free(line);
  }

  if (fclose(file) != 0) {
    perror("Failed to close file");
    return -1;
  }

  fprintf(stdout, "Successfully received and wrote %zu CSV lines to %s\n",
          lines_received, filename);
  return 0;
}
