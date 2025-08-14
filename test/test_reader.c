#include "file_reader.h"
#include "hash_table.h"
#include "word.h"
#include <fcntl.h>
#include <stdio.h>
#include <unistd.h>

int main() {
  // Apri file di test
  int fd = open("test_files/test_reader_utf8.txt", O_RDONLY);
  if (fd < 0) {
    perror("open");
    return 1;
  }
  lseek(fd, 0, SEEK_SET); // Assicurati di partire dall'inizio del file
  // Test lettura parola per parola
  printf("=== Test lettura parole ===\n");
  int word_buf[MAX_WORD_LENGTH];
  while (f_get_next_word(fd, word_buf) == 0) {
    if (word_buf[0] == 0)
      continue; // salta parole vuote
    word_t *w = create_word(word_buf);
    if (w) {
      free_word(w);
    }
    move_cursor_to_next_word(fd);
  }

  // Riavvolgi file e costruisci hash table
  lseek(fd, 0, SEEK_SET);
  hash_table_t *table = build_word_hashtable_fd(fd);
  if (table) {
    printf("\n=== Hash table parole ===\n");
//    print_hash_table(table,
//                    STDOUT_FILENO); // stampa usando la tua funzione di stampa
//    int fd1 = open("test_files/out_test_reader_utf8.txt", O_WRONLY | O_CREAT | O_TRUNC, 0644);
//    print_hash_table(table, fd1);
    free_hash_table(table);
  }

  close(fd);
  return 0;
}
