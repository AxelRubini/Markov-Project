#ifndef UTF8_TOOLS_H
#define UTF8_TOOLS_H
#include "utils.h"
#include <stdint.h>

// error codes for UTF-8 reading bad formats
#define UTF8_ERROR (-2)

/**
 * @brief Read a UTF-8 code point from stdin.
 *
 * @return Code point, EOF on end, or UTF8_ERROR on malformed input.
 */
int utf8_getchar(void);

/**
 * @brief Read a UTF-8 code point from a descriptor.
 *
 * @param fd File descriptor.
 * @param bytes_read Output byte count.
 * @return Code point, EOF on end, or UTF8_ERROR on malformed input.
 */
int utf8_getchar_fd(int fd, int *bytes_read);

/**
 * @brief Fetch a UTF-8 code point from a buffer at an offset.
 *
 * @param buf Byte buffer.
 * @param offset In/out offset.
 * @return Code point, EOF on end, or UTF8_ERROR on malformed input.
 */
int utf8_getchar_from_buf(const u8 *buf, size_t *offset);

/**
 * @brief Write a Unicode code point to a descriptor in UTF-8.
 *
 * @param codepoint Unicode code point.
 * @param fd Destination descriptor.
 */
void utf8_putchar(int codepoint, int fd);

/**
 * @brief Convert a code point to lowercase.
 *
 * @param codepoint Input code point.
 * @return Lowercase code point.
 */
int utf8_char_to_lower(int codepoint);

/**
 * @brief Convert a code point to uppercase.
 *
 * @param codepoint Input code point.
 * @return Uppercase code point.
 */
int utf8_char_to_upper(int codepoint);

/**
 * @brief Print a UTF-8 word to a descriptor.
 *
 * @param word Array of code points terminated by 0.
 * @param fd Destination descriptor.
 */
void utf8_print_word(const int *word, int fd);

/**
 * @brief Convert a word to lowercase and free the original.
 *
 * @param word Array of code points.
 * @param word_length Number of code points including terminator.
 * @return Newly allocated lowercase word or NULL on error.
 */
int *utf8_word_to_lower(const int *word, int word_length);

/**
 * @brief Append a code point to a UTF-8 buffer.
 *
 * @param codepoint Code point to append.
 * @param buffer Destination buffer.
 * @param offset In/out byte offset.
 */
void utf8_append_char(uint32_t codepoint, char *buffer, int *offset);

/**
 * @brief Compare two UTF-8 strings.
 *
 * @param str1 First string of code points.
 * @param str2 Second string of code points.
 * @return 0 if equal, non-zero otherwise.
 */
int utf8_strcmp(const int *str1, const int *str2);

/**
 * @brief Deep copy a UTF-8 string.
 *
 * @param str Source string of code points.
 * @return Newly allocated copy or NULL on error.
 */
int *utf8_str_deepcopy(const int *str);

/**
 * @brief Extract a code point from a raw UTF-8 buffer.
 *
 * @param buffer Byte buffer.
 * @param offset In/out offset.
 * @return Code point or -1 on error.
 */
int utf8_get_codepoint(const char *buffer, int *offset);

/**
 * @brief Encode code points into raw UTF-8 bytes.
 *
 * @param word Array of code points.
 * @param buf Output byte buffer.
 * @return Bytes written or -1 on error.
 */
int utf8_cp_to_rawbytes(const int *word, u8 *buf);

/**
 * @brief Return non-zero if code point is ignorable punctuation.
 */
int is_punctuation(int cp);

/**
 * @brief Return non-zero if code point terminates a sentence.
 */
int is_sentence_terminator(int cp);

#endif
