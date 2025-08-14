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

/**
 * @brief Check if a number is prime.
 *
 * @param n Value to test.
 * @return 1 if prime, 0 otherwise.
 */
unsigned int is_prime(int n);

/**
 * @brief Compute the next prime greater or equal to n.
 *
 * @param n Starting value.
 * @return Next prime.
 */
unsigned int next_prime(int n);

/**
 * @brief Hash a word for a table size.
 *
 * @param key Array of code points.
 * @param table_size Number of buckets.
 * @return Hash index.
 */
unsigned int hash_function(const int *key, int table_size);

/**
 * @brief Estimate probability of a follower word.
 *
 * @param word Occurrence count of the current word.
 * @param next_word Occurrence count of the follower.
 * @return Probability in range [0,1].
 */
double next_word_probability(const int *word, const int *next_word);

/**
 * @brief Allocate zeroed memory or abort.
 *
 * @param size Bytes to allocate.
 * @return Pointer to cleared memory.
 */
void *dmalloc(size_t size);

/**
 * @brief Write all bytes to a descriptor, retrying on EINTR.
 *
 * @param fd Destination file descriptor.
 * @param buf Data buffer.
 * @param len Number of bytes to write.
 * @return 0 on success, -1 on error.
 */
int write_all(int fd, const void *buf, size_t len);

/**
 * @brief Read an exact number of bytes from a descriptor.
 *
 * @param fd Source file descriptor.
 * @param buf Destination buffer.
 * @param len Number of bytes to read.
 * @return Bytes read or -1 on error.
 */
int read_all(int fd, void *buf, size_t len);

/**
 * @brief Send a framed payload.
 *
 * @param fd Destination descriptor.
 * @param data_to_send Payload data.
 * @param len Payload length in bytes.
 * @return 0 on success, -1 on error.
 */
int send_frame(int fd, const char *data_to_send, size_t len);

/**
 * @brief Receive a framed payload, allocating buffer as needed.
 *
 * @param fd Source descriptor.
 * @param buf Pointer to buffer pointer.
 * @param cap Pointer to buffer capacity.
 * @return Bytes received, 0 on EOF, or -1 on error.
 */
ssize_t recv_frame(int fd, char **buf, size_t *cap);
#endif
