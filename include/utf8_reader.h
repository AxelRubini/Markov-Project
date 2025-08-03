#ifndef UTF8_READER_H
#define UTF8_READER_H

#include <stdint.h>

// error codes for UTF-8 reading bad formats
#define UTF8_ERROR (-2)

/*
 * The function is a substitute for getchar() that reads a UTF-8 encoded character from the standard input.
 * returns the Unicode of the character read, or UTF8_ERROR if the input is not a valid UTF-8 sequence.
 * */
int utf8_getchar(void);
#endif
