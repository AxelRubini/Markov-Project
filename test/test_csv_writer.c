#include "CSV_writer.h"
#include "file_reader.h"
#include "hash_table.h"
#include "word.h"
#include <fcntl.h>
#include <stdio.h>
#include <unistd.h>

int main(void) {
  // open test file
  int fd = open("test_files/test_reader_utf8.txt", O_RDONLY);
  if (fd < 0) {
    perror("open");
    return 1;
  }
  lseek(fd, 0, SEEK_SET); // ensure start from file beginning
  // test reading word by word
  printf("=== Test reading words ===\n");
  int word_buf[MAX_WORD_LENGTH];
  while (f_get_next_word(fd, word_buf) == 0) {
    if (word_buf[0] == 0)
      continue; // skip empty words
    word_t *w = create_word(word_buf);
    if (w) {
      free_word(w);
    }
    move_cursor_to_next_word(fd);
  }
  // rewind file and build hash table
  lseek(fd, 0, SEEK_SET);
  hash_table_t *table = build_word_hashtable_fd(fd);
  const char *output_filename = "test_files/out_test_reader_utf8.csv";
  csv_build_file(output_filename, table);
  free_hash_table(table);
  close(fd);
  return 0;
}
