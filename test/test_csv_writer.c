#include "../include/CSV_writer.h"
#include "../include/file_reader.h"
#include "../include/hash_table.h"
#include "../include/word.h"
#include <fcntl.h>
#include <stdio.h>
#include <unistd.h>






int main(void) {
  // Apri file di test
  int fd = open("test/test_files/test_reader_utf8.txt", O_RDONLY);
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
      printf("Parola: ");
      fflush(stdout);                    // Assicura che il buffer sia svuotato     
      print_utf8_word(w, STDOUT_FILENO); // stampa come testo
      printf("\n");
      free_word(w);
    }
    move_cursor_to_next_word(fd);
  }
  // Riavvolgi file e costruisci hash table
  lseek(fd, 0, SEEK_SET);
  hash_table_t *table = build_word_hashtable_fd(fd);
  const char *output_filename = "test/test_files/out_test_reader_utf8.csv";
  csv_build_file(output_filename, table);
  free_hash_table(table);
  close(fd);
  return 0;
}

