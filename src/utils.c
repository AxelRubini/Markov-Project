#include "../include/utils.h"

#include <arpa/inet.h>
#include <errno.h>
#include <math.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>

unsigned int is_prime(int n) {

  if (n <= 1)
    return 0;
  if (n <= 3)
    return 1;
  int max_to_divide = (int)sqrt(n);
  for (int i = 2; i <= max_to_divide; i++) {
    if (n % i == 0) {
      return 0; // not prime
    }
  }
  return 1; // prime
}

unsigned int next_prime(int n) {
  if (n <= 1)
    return 2;
  if (n % 2 == 0)
    n++; // make it odd
  while (!is_prime(n)) {
    n += 2; // check only odd numbers
  }
  return n;
}

unsigned int hash_function(const int *key, int table_size) {

  unsigned int hash = 5381;
  int c;

  while ((c = *key++)) {
    hash = ((hash << 5) + hash) + c; // hash * 33 + c
  }

  return hash % table_size;
}

void *dmalloc(size_t size) {
  void *ptr = malloc(size);
  if (ptr == NULL) {
    fprintf(stderr, "Memory allocation failed\n");
    exit(EXIT_FAILURE);
  }
  memset(ptr, 0, size); // Initialize allocated memory to zero
  return ptr;
}
#define malloc(x) dont_use_malloc_usedmalloc // Use dmalloc instead
//
double next_word_probability(const int *word, const int *next_word) {
  double probability = 0.0;
  probability = (double)(*next_word) / (double)(*word); // Example calculation
  return probability;
}
int write_all(int fd, const void *buf, size_t len) {
  const char *ptr = buf; // bytes to send
  size_t left = len;
  while (left > 0) {
    ssize_t written = write(fd, ptr, left);
    if (written < 0) {
      if (errno == EINTR) {
        continue; // Interrupted, try again
                  //     }
        perror("write_all");
        return -1; // Error writing
      }
    }
    left -= written;
    ptr += written;
  }

  return 0; // All bytes written successfully
}

int read_all(int fd, void *buf, size_t len) {
  char *ptr = buf; // Destination buffer
  size_t left = len;
  while (left > 0) {
    ssize_t read_bytes = read(fd, ptr, left);
    if (read_bytes < 0) {
      if (errno == EINTR) {
        continue; // Interrupted, try again
      }
      perror("read_all");
      return -1; // Error reading
    }
    if (read_bytes == 0) {
      break; // End of file reached
    }
    left -= read_bytes;
    ptr += read_bytes;
  }

  return len - left; // Return number of bytes read
}

int send_frame(int fd, const char *data_to_send, size_t len) {
  if (data_to_send == NULL || len == 0) {
    fprintf(stderr, "Invalid data to send or length is zero\n");
    return -1;
  }
  /*Header:
   * The first 4 bytes of the frame contain the length of the data that follows.
   * This allows the receiver to know how many bytes to expect.
   */
  uint32_t frame_length =
      htonl((uint32_t)len); // Convert the length to network byte order so the
                            // receiver can interpret the length correctly
  if (write_all(fd, &frame_length, sizeof(frame_length)) <
      0) { // Write the frame length first
    perror("send_frame: write frame length");
    return -1;
  }
  if (len == 0) {
    return 0; // No data to send, just sent the frame length
  }
  if (write_all(fd, data_to_send, len) < 0) { // Write the actual data
    perror("send_frame: write data");
    return -1;
  }
  return 0;
}

ssize_t receive_frame(int fd, char **buf, size_t *cap) {
  if (!buf || !cap) {
    errno = EINVAL;
    return -1;
  }
  /* Read the first 4 bytes to get the length of the data that follows.
   * This is the frame header.
   */
  uint32_t be_len = 0;
  int st = read_all(fd, &be_len, sizeof(be_len));
  if (st == 1)
    return 0;
  if (st < 0)
    return -1;

  uint32_t n =
      ntohl(be_len);       // Convert from network byte order to host byte order
  size_t need = (size_t)n; // Number of bytes needed for the data

  if (*buf == NULL ||
      *cap <
          need +
              1) { // +1 per '\0' // check if buffer is NULL or not large enough
    size_t ncap = need + 1;
    char *nb = (char *)realloc(*buf, ncap);
    if (!nb)
      return -1;
    *buf = nb;
    *cap = ncap;
  }
  if (need == 0) {
    (*buf)[0] = '\0';
    return 1;
  }

  st = read_all(fd, *buf, need);
  if (st == 1)
    return -1;
  if (st < 0)
    return -1;

  (*buf)[need] = '\0';
  return (ssize_t)need;
}
