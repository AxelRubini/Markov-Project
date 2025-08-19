#ifndef UTILS_H
#define UTILS_H

#include <math.h>
#include <stdint.h>
#include <stdlib.h>

#define MAX_WORD_LENGTH_NO_BUF                                                 \
  30 // Maximum length of a word in code points with null terminator
#define FLOATING_POINT_CHAR_SPACE 24
#define MAX_WORD_LENGTH                                                        \
  31 // Maximum length of a word in code points without null terminator
#define MAX_WORD_LENGTH_BYTES                                                  \
  ((MAX_WORD_LENGTH - 1) * 4 +                                                 \
   1) // Maximum length of a word in bytes (UTF-8) with null terminator

#define ARR_SZ(arr) (sizeof(arr) / sizeof((arr)[0]))

typedef uint8_t u8;
typedef uint32_t u32;

/**
 * @brief Check if a number is prime.
 *
 * @param n Value to test.
 * @return 1 if prime, 0 otherwise.
 */
uint32_t is_prime(int n);

/**
 * @brief Compute the next prime greater or equal to n.
 *
 * @param n Starting value.
 * @return Next prime.
 */
uint32_t next_prime(int n);

/**
 * @brief Hash a word for a table size.
 *
 * @param key Array of code points.
 * @param table_size Number of buckets.
 * @return Hash index.
 */
uint32_t hash_function(const int *key, int table_size);

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
 * @brief Close an fd safely.
 * @param fd File descriptor to close.
 * @return 0 on success, -1 on error.
 */
int safe_close(int *fd);

/**
 * @brief Close a series of pipes safely.
 * @param fds Array of file descriptors to close.
 * @param count Number of file descriptors.
 * @return 0 on success, -1 on error.
 */
int safe_close_pipes(int *fds, size_t count);

#endif
