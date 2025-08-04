#include "../include/utf8_tools.h"
#include <stdio.h>
#include <unistd.h>

/*
 * Expected length of character sequence referenced by the leading byte.
 * return 1 for ASCII characters (ASCII is Subset of UTF-8)
 * return 2 for 2-byte sequences, 3 for 3-byte sequences, and
 * 4 for 4-byte sequences.
 */
static unsigned utf8_length(unsigned char lead) {
  if (lead < 0x80)
    return 1; // 0xxxxxxx
  if ((lead >> 5) == 0x6)
    return 2; // 110xxxxx
  if ((lead >> 4) == 0xE)
    return 3; // 1110xxxx
  if ((lead >> 3) == 0x1E)
    return 4; // 11110xxx
  return 0;   // invalid
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

int utf8_getchar_fd(int fd) {
  unsigned char lead;
  if (read(fd, &lead, 1) != 1) {
    return EOF; // end of input
  }

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
    if (read(fd, &lead, 1) != 1) // Read the next byte
      return EOF;                // End of input before completing the character
    if ((lead >> 6) != 0x2)
      return UTF8_ERROR; // invalid continuation byte
    codepoint =
        (codepoint << 6) | (lead & 0x3F); // append the continuation byte
  }

  return (int)codepoint;
}

void utf8_putchar(int codepoint, int fd) {
  if (codepoint < 0 || codepoint > 0x10FFFF) {
    return; // Invalid codepoint
  }

  unsigned char buffer[4];
  unsigned len = 0;

  if (codepoint < 0x80) {
    buffer[len++] = (unsigned char)codepoint; // 1-byte sequence
  } else if (codepoint < 0x800) {
    buffer[len++] = (unsigned char)((codepoint >> 6) | 0xC0); // 2-byte sequence
    buffer[len++] = (unsigned char)((codepoint & 0x3F) | 0x80);
  } else if (codepoint < 0x10000) {
    buffer[len++] =
        (unsigned char)((codepoint >> 12) | 0xE0); // 3-byte sequence
    buffer[len++] = (unsigned char)(((codepoint >> 6) & 0x3F) | 0x80);
    buffer[len++] = (unsigned char)((codepoint & 0x3F) | 0x80);
  } else {
    buffer[len++] =
        (unsigned char)((codepoint >> 18) | 0xF0); // 4-byte sequence
    buffer[len++] = (unsigned char)(((codepoint >> 12) & 0x3F) | 0x80);
    buffer[len++] = (unsigned char)(((codepoint >> 6) & 0x3F) | 0x80);
    buffer[len++] = (unsigned char)((codepoint & 0x3F) | 0x80);
  }

  write(fd, buffer, len); // Write the UTF-8 sequence to the file descriptor
}

int utf8_char_to_lower(int codepoint) {
  // Handle ASCII characters
  if (codepoint >= 'A' && codepoint <= 'Z') {
    return codepoint + ('a' - 'A');
  }

  // Handle specific Unicode characters
  switch (codepoint) {
  case 0x0116:
    return 0x0117; // Ė -> ė
  default:
    return codepoint; // No lowercase equivalent found, return original
                      // codepoint
  }
}

int utf8_char_to_upper(int codepoint) {
  // Handle ASCII characters
  if (codepoint >= 'a' && codepoint <= 'z') {
    return codepoint - ('a' - 'A');
  }

  // Handle specific Unicode characters
  switch (codepoint) {
  case 0x0117:
    return 0x0116; // ė -> Ė
  default:
    return codepoint; // No uppercase equivalent found, return original
                      // codepoint
  }
}

void utf8_print_word(const int **word, int fd) {
  if (word == NULL || *word == NULL) {
    return; // Nothing to print
  }

  const int *current = *word;
  while (*current != 0) { // Assuming the word is null-terminated
    utf8_putchar(*current, fd);
    current++;
  }
}
