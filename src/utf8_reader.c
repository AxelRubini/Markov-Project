#include "../include/utf8_reader.h"
#include <stdio.h>

/*
 * Hacker's Delight, 2nd edition, by Henry S. Warren, Jr.
 * Chapter 5, page 79
 * counts the number of leading zeros in a byte
 */
static unsigned num_of_Leading_zeros(unsigned char x) {
  unsigned b = 0;
  if (!(x & 0xF0)) {
    x <<= 4;
    b += 4;
  }
  if (!(x & 0xC0)) {
    x <<= 2;
    b += 2;
  }
  if (!(x & 0x80)) {
    b += 1;
  }
  return b;
}

/*
 * Expected length of character sequence referenced by the leading byte.
 * return 1 for ASCII characters (ASCII is Subset of UTF-8)
 * return 2 for 2-byte sequences, 3 for 3-byte sequences, and
 * 4 for 4-byte sequences.
 */
static unsigned utf8_length(unsigned char lead) {
  if (lead < 0x80) // is ASCII
    return 1;
  return num_of_Leading_zeros(lead);
}
/*
 * https://en.wikipedia.org/wiki/UTF-8
 * https://www.youtube.com/watch?v=c_hfKgektt4
 * https://stackoverflow.com/questions/68835028/utf-8-hex-to-unicode-code-point-only-math
 */
int utf8_getchar(void) {
  int c = getchar();
  if (c == EOF) {
    return EOF; // end of input
  }
  unsigned char lead = (unsigned char)c;

  unsigned len = utf8_length(lead);
  if (len == 1) {
    return lead; // ASCII character
  }

  if (len < 2 || len > 4) {
    return UTF8_ERROR; // invalid leading byte
  }

  uint32_t codepoint =
      lead & ((1u << (7 - len)) - 1); // Mask to get the leading bits
  for (unsigned i = 1; i < len; ++i) {
    int ci = getchar();
    if (ci == EOF)
      return EOF; // End of input before completing the character
    unsigned char byte = (unsigned char)ci;
    if ((byte >> 6) != 0x2)
      return UTF8_ERROR; // invalid continuation byte
    codepoint =
        (codepoint << 6) | (byte & 0x3F); // append the continuation byte
  }

  return (int)codepoint;
}
