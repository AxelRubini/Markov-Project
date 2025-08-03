#ifndef UTF8_TOOLS_H
#define UTF8_TOOLS_H

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
 * The function reads from the specified file descriptor and returns a Unicode codepoint.
 */
int utf8_getchar_fd(int fd);

/*
 * the function reads a Unicode codepoint and encodes it as a UTF-8 sequence,
 * and writes it to the specified file descriptor/ fd: file descriptor to write
 * to codepoint: the Unicode codepoint to encode
 */
void utf8_putchar(int codepoint, int fd);



#endif
