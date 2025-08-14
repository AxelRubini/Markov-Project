#include "utf8_tools.h"
#include "utils.h"
#include <stdio.h>
#include <unistd.h>

/*
 * Expected length of character sequence referenced by the leading byte.
 * return 1 for ASCII characters (ASCII is Subset of UTF-8)
 * return 2 for 2-byte sequences, 3 for 3-byte sequences, and
 * 4 for 4-byte sequences.
 */
static unsigned utf8_length(u8 lead) {
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
 * https://utf8everywhere.org/
 */
int utf8_getchar(void) {
  int c = getchar();
  if (c == EOF) {
    return EOF; // end of input
  }
  u8 lead = (u8)c;

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
    u8 byte = (u8)ci;
    if ((byte >> 6) != 0x2)
      return UTF8_ERROR; // invalid continuation byte
    codepoint =
        (codepoint << 6) | (byte & 0x3F); // append the continuation byte
  }

  return (int)codepoint;
}

int utf8_getchar_fd(int fd, int *bytes_read) {
  u8 lead;
  if (read(fd, &lead, 1) != 1) {
    return EOF; // end of input
  }

  *bytes_read = 1;
  unsigned len = utf8_length(lead);
  if (len == 1) {
    return lead; // ASCII
  }

  if (len < 2 || len > 4) {
    return UTF8_ERROR; // invalid leading byte
  }

  uint32_t codepoint = lead & ((1u << (7 - len)) - 1);
  for (unsigned i = 1; i < len; ++i) {
    if (read(fd, &lead, 1) != 1)
      return EOF;
    if ((lead >> 6) != 0x2)
      return UTF8_ERROR;
    codepoint = (codepoint << 6) | (lead & 0x3F);
    (*bytes_read)++;
  }

  return (int)codepoint;
}

int utf8_getchar_from_buf(const u8 *buf, size_t *offset) {
  if (!buf || !offset)
    return UTF8_ERROR;

  u8 lead = buf[*offset];
  if (lead == '\0')
    return EOF; // end of buffer

  uint32_t len = utf8_length(lead); // expect 1..4
  if (len < 1 || len > 4)
    return UTF8_ERROR;

  // Fast path: ASCII
  if (len == 1) {
    (*offset) += 1;
    return (int)lead;
  }

  uint32_t cp = lead & ((1u << (7 - len)) - 1);

  // Validate continuation bytes and assemble code point
  for (unsigned i = 1; i < len; ++i) {
    u8 c = buf[*offset + i];
    if (c == '\0')
      return EOF; // truncated at end
    if ((c & 0xC0) != 0x80)
      return UTF8_ERROR; // not 10xxxxxx
    cp = (cp << 6) | (uint32_t)(c & 0x3F);
  }

  (*offset) += len;
  return (int)cp;
}

void utf8_putchar(int codepoint, int fd) {
  if (codepoint < 0 || codepoint > 0x10FFFF) {
    return; // Invalid codepoint
  }

  u8 buffer[4];
  unsigned len = 0;

  if (codepoint < 0x80) {
    buffer[len++] = (u8)codepoint; // 1-byte sequence
  } else if (codepoint < 0x800) {
    buffer[len++] = (u8)((codepoint >> 6) | 0xC0); // 2-byte sequence
    buffer[len++] = (u8)((codepoint & 0x3F) | 0x80);
  } else if (codepoint < 0x10000) {
    buffer[len++] = (u8)((codepoint >> 12) | 0xE0); // 3-byte sequence
    buffer[len++] = (u8)(((codepoint >> 6) & 0x3F) | 0x80);
    buffer[len++] = (u8)((codepoint & 0x3F) | 0x80);
  } else {
    buffer[len++] = (u8)((codepoint >> 18) | 0xF0); // 4-byte sequence
    buffer[len++] = (u8)(((codepoint >> 12) & 0x3F) | 0x80);
    buffer[len++] = (u8)(((codepoint >> 6) & 0x3F) | 0x80);
    buffer[len++] = (u8)((codepoint & 0x3F) | 0x80);
  }

  write(fd, buffer, len); // Write the UTF-8 sequence to the file descriptor
}

int utf8_char_to_lower(int codepoint) {
  // Handle ASCII characters
  if (codepoint >= 'A' && codepoint <= 'Z') {
    return codepoint + ('a' - 'A');
  }

  // Specific Unicode characters for italian accented Lowercase
  switch (codepoint) {
  case 200:
    return 232;
  case 201:
    return 233;
  default:
    return codepoint; // No italian special character keep it like that
                      // codepoint
  }
}

int utf8_char_to_upper(int codepoint) {
  // Handle ASCII characters
  if (codepoint >= 'a' && codepoint <= 'z') {
    return codepoint - ('a' - 'A');
  }

  // Specific Unicode characters for italian accented Uppercase
  switch (codepoint) {
  case 232:
    return 200;
  case 233:
    return 201;
  default:
    return codepoint; // not italian special characther keep it like that //
                      // codepoint
  }
}

void utf8_print_word(const int *word, int fd) {
  if (word == NULL) {
    fprintf(stderr, "Word is NULL\n");
    return;
  }

  for (int i = 0; word[i] != '\0'; i++) {
    utf8_putchar(word[i], fd);
  }
}

int *utf8_word_to_lower(const int *word, int word_length) {
  if (word == NULL) {
    fprintf(stderr, "Word is NULL\n");
    return NULL;
  }

  int *lower_word =
      dmalloc(sizeof(int) * (word_length)); // Allocate memory for the new word
  int i = 0;

  while (word[i] != '\0') {
    lower_word[i] = utf8_char_to_lower(word[i]);
    i++;
  }
  lower_word[i] = '\0'; // Null-terminate the new word

  return lower_word;
}

void utf8_append_char(unsigned int codepoint, char *buffer, int *offset) {
  if (codepoint < 0x80) {
    buffer[(*offset)++] = (u8)codepoint; // 1-byte sequence
  } else if (codepoint < 0x800) {
    buffer[(*offset)++] = (u8)((codepoint >> 6) | 0xC0); // 2-byte sequence
    buffer[(*offset)++] = (u8)((codepoint & 0x3F) | 0x80);
  } else if (codepoint < 0x10000) {
    buffer[(*offset)++] = (u8)((codepoint >> 12) | 0xE0); // 3-byte sequence
    buffer[(*offset)++] = (u8)(((codepoint >> 6) & 0x3F) | 0x80);
    buffer[(*offset)++] = (u8)((codepoint & 0x3F) | 0x80);
  } else {
    buffer[(*offset)++] = (u8)((codepoint >> 18) | 0xF0); // 4-byte sequence
    buffer[(*offset)++] = (u8)(((codepoint >> 12) & 0x3F) | 0x80);
    buffer[(*offset)++] = (u8)(((codepoint >> 6) & 0x3F) | 0x80);
    buffer[(*offset)++] = (u8)((codepoint & 0x3F) | 0x80);
  }

  return;
}

int utf8_strcmp(const int *str1, const int *str2) {
  if (str1 == NULL || str2 == NULL) {
    return (str1 == str2) ? 0 : (str1 ? 1 : -1);
  }
  int i = 0;
  while (str1[i] != '\0' && str2[i] != '\0') {
    if (str1[i] != str2[i]) {
      return str1[i] - str2[i];
    }
    i++;
  }
  return str1[i] - str2[i];
}

int *utf8_str_deepcopy(const int *str) {
  if (str == NULL) {
    return NULL;
  }

  int length = 0;
  while (str[length] != '\0') {
    length++;
  }

  int *copy = dmalloc(sizeof(int) * (length + 1));
  for (int i = 0; i < length; i++) {
    copy[i] = str[i];
  }
  copy[length] = '\0'; // Null-terminate the copied string

  return copy;
}

int utf8_cp_to_rawbytes(const int *word, u8 *buf) {
  if (word == NULL || buf == NULL) {
    fprintf(stderr, "Invalid arguments to utf8_cp_to_rawbytes\n");
    return -1; // Error
  }

  int offset = 0;
  for (int i = 0; word[i] != '\0'; i++) {
    utf8_append_char(word[i], (char *)buf, &offset);
  }
  buf[offset] = '\0'; // Null-terminate the buffer

  return offset; // Return the number of bytes written

}
