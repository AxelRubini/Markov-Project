#ifndef UTF8_TOOLS_H
#define UTF8_TOOLS_H
/*
 * This header file provides functions for reading and writing UTF-8 encoded
 * characters. functions like strcopy,strcmp can be used to manipulate UTF-8
 * strings, because they read and compare bytes.
 */
#include <stdint.h>

// error codes for UTF-8 reading bad formats
#define UTF8_ERROR (-2)

/*
 * The function is a substitute for getchar() that reads a UTF-8 encoded
 * character from the standard input. returns the Unicode of the character read,
 * or UTF8_ERROR if the input is not a valid UTF-8 sequence.
 * */
int utf8_getchar(void);
/*
 * The function reads from the specified file descriptor and returns a Unicode
 * codepoint and the number of bytes readed.
 */
int utf8_getchar_fd(int fd, int *bytes_read);

/*
 * the function reads a Unicode codepoint and encodes it as a UTF-8 sequence,
 * and writes it to the specified file descriptor/ fd: file descriptor to write
 * to codepoint: the Unicode codepoint to encode
 */
void utf8_putchar(int codepoint, int fd);

/*
 * The function reads a UTF-8 codepoint and returns the lowercase equivalent
 * If the codepoint does not have a lowercase equivalent, it returns the
 * original codepoint.
 */
int utf8_char_to_lower(int codepoint);

/*
 * The function reads a UTF-8 codepoint and returns the uppercase equivalent
 * If the codepoint does not have an uppercase equivalent, it returns the
 * original codepoint.
 */
int utf8_char_to_upper(int codepoint);

/*
 * the function takes a UTF-8 encoded word (array of codepoints) and prints it 
 * to the specified file descriptor.
 * parameters:
 * word: pointer to the first codepoint of the word (integer array)
 * fd: file descriptor to write to
 * NOTE: the function does not add a newline at the end of the word
 */

void utf8_print_word(const int *word, int fd);

/*
 * The function takes a UTF-8 encoded word (array of codepoints) and converts
 * it to lowercase.
 * it returns a new allocated array of codepoints representing the lowercase
 * version of the word
 * NOTE: the function put the null terminator at the end of the word
 * NOTE: the original word memory is freed by the function
 */

int *utf8_word_to_lower(const int *word, int word_length);

/*
 * The function takes a Unicode codepoint and appends it to the provided buffer
 * in UTF-8 encoding. It updates the offset to reflect the new position in the
 * buffer.
 * parameters:
 * codepoint: the Unicode codepoint to append
 * buffer: pointer to the buffer where the UTF-8 sequence will be written
 * offset: pointer to an integer that will be updated with the new offset
 * returns: the number of bytes written to the buffer
 */
void utf8_append_char(unsigned int codepoint, char *buffer, int *offset);


/*
 * The function compares two UTF-8 encoded strings (arrays of codepoints) and
 * returns an integer less than, equal to, or greater than zero if the first 
 * string is found.
 * less than = Not equal
 * equal to = Match
 * greater than = Not equal
 * Parameters:
 * str1: pointer to the first codepoint of the first string (integer array)
 * str2: pointer to the first codepoint of the second string (integer array)
 * Returns: an integer indicating the result of the comparison
 */
int utf8_strcmp(const int *str1, const int *str2);


/*
 * The function creates a deep copy of a UTF-8 encoded string (array of codepoints).
 * It allocates new memory for the copied string and returns a pointer to it.
 * Parameters:
 * str: pointer to the first codepoint of the original string (integer array)
 * Returns: a pointer to the newly allocated copy of the string
 */
int *utf8_str_deepcopy(const int *str);




#endif
