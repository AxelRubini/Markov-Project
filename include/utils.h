#ifndef UTILS_H
#define UTILS_H

#include <math.h>
#include <stdlib.h>
#include <sys/types.h>

#define MAX_WORD_LENGTH_NO_BUF 30 // Maximum length of a word in code points with null terminator
#define FLOATING_POINT_CHAR_SPACE 24
#define MAX_WORD_LENGTH 31 // Maximum length of a word in code points without null terminator
#define MAX_WORD_LENGTH_BYTES ((MAX_WORD_LENGTH - 1) * 4 +1) // Maximum length of a word in bytes (UTF-8) with null terminator

typedef unsigned char u8;
typedef unsigned int u32;
unsigned int is_prime(int n);
unsigned int next_prime(int n);
unsigned int hash_function(const int *key, int table_size);
double next_word_probability(const int *word, const int *next_word);
void *dmalloc(size_t size);
/*
 * Write all ensure that all bytes are written to the file descriptor.
 * it will retry if interrupted by a signal.
 * returns if successful, or -1 on error.
 * parameters:
 *  fd: file descriptor to write to
 *  buf: buffer to write
 *  len: number of bytes to write
 */
int write_all(int fd, const void *buf, size_t len);
/*
 * Read all ensure that all bytes are read from the file descriptor.
 * it will retry if interrupted by a signal.
 * returns: number of bytes read, or -1 on error.
 * parameters:
 *  fd: file descriptor to read from
 *  buf: buffer to read into
 *  len: number of bytes to read
 */
int read_all(int fd, void *buf, size_t len);

/* * Send a frame over the file descriptor.
 * It sends the length of the data first, then the data itself.
 * returns: 0 on success, -1 on error.
 * parameters:
 *  fd: file descriptor to send data to
 *  data_to_send: pointer to the data to send
 *  len: length of the data to send
 */
int send_frame(int fd, const char *data_to_send, size_t len);

/*
 * Receive a frame from the file descriptor.
 * It reads the length of the data first, then the data itself.
 * The buffer is allocated or reallocated as needed.
 * returns: number of bytes read, or -1 on error, or 0 if end of file reached.
 * parameters:
 *  fd: file descriptor to receive data from
 *  buf: pointer to the buffer to store the received data
 *  cap: pointer to the capacity of the buffer
 */
ssize_t recv_frame(int fd, char **buf, size_t *cap);
#endif
