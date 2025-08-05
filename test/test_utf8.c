#include"../include/utf8_tools.h"
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

void print_utf8_char(int codepoint) {
    unsigned char utf8[5] = {0}; // max 4 byte + terminatore
    if (codepoint <= 0x7F) {
        utf8[0] = codepoint;
    } else if (codepoint <= 0x7FF) {
        utf8[0] = 0xC0 | (codepoint >> 6);
        utf8[1] = 0x80 | (codepoint & 0x3F);
    } else if (codepoint <= 0xFFFF) {
        utf8[0] = 0xE0 | (codepoint >> 12);
        utf8[1] = 0x80 | ((codepoint >> 6) & 0x3F);
        utf8[2] = 0x80 | (codepoint & 0x3F);
    } else {
        utf8[0] = 0xF0 | (codepoint >> 18);
        utf8[1] = 0x80 | ((codepoint >> 12) & 0x3F);
        utf8[2] = 0x80 | ((codepoint >> 6) & 0x3F);
        utf8[3] = 0x80 | (codepoint & 0x3F);
    }
    printf("%s", utf8);
}
int main() {
    FILE *file = freopen("test/test_files/test_file_utf8.txt", "rb", stdin);
    if (!file) {
        fprintf(stderr, "Could not open file\n");
        return 1;
    }
    FILE *output_file = freopen("test/test_files/output_utf8.txt", "w+", stdout);
    if (!output_file) {
        fprintf(stderr, "Could not open output file\n");
        fclose(file);
        return 1;
    }
    int c;
    while ((c = utf8_getchar()) != EOF) {
        if (c == UTF8_ERROR) {
            fprintf(stderr, "Invalid UTF-8 sequence encountered\n");
            fclose(file);
            return 1;
        }
        // Print the Unicode code point in hexadecimal format
        printf("U+%04X\n", c);
        print_utf8_char(c);
        printf("\n");

        fprintf(stderr, "DEBUG: codepoint=%d (0x%X)\n", c, c);

    }
    return 0;
}
