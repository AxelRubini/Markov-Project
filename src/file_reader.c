#include "../include/file_reader.h"
#include "../include/utf8_tools.h"

#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>
hash_table_t *build_word_hashtable_fd(int fd) {
  if (fd < 0) {
    fprintf(stderr, "Invalid file descriptor\n");
    return NULL;
  }

  hash_table_t *table =
      create_hash_table(HASH_TABLE_SIZE, word_hash, word_hashtable_keycmp);
  if (table == NULL) {
    fprintf(stderr, "Failed to create hash table\n");
    return NULL;
  }

  int current_word[MAX_WORD_LENGTH] = {0}; // buffer for the current word
  int next_word[MAX_WORD_LENGTH] = {0};    // buffer for the next word

  // Leggi la prima parola
  move_cursor_to_next_word(fd);
  int status = f_get_next_word(fd, current_word);

  if (status == 1) {
    // File vuoto
    return table;
  }
  if (status == -1) {
    // Errore nella lettura
    free_hash_table(table);
    return NULL;
  }

  // Loop principale per leggere coppie di parole
  while (1) {
    // Verifica che current_word sia valida
    if (current_word[0] == 0) {
      // Parola vuota, prova a leggere la prossima
      move_cursor_to_next_word(fd);
      status = f_get_next_word(fd, current_word);
      if (status != 0)
        break;
      continue;
    }

    // Crea word_t per current_word
    word_t *word = create_word(current_word);
    if (word == NULL) {
      fprintf(stderr, "Failed to create word from current_word\n");
      // Prova a continuare con la prossima parola
      move_cursor_to_next_word(fd);
      status = f_get_next_word(fd, current_word);
      if (status != 0)
        break;
      continue;
    }

    // Leggi la parola successiva
    move_cursor_to_next_word(fd);
    int next_status = f_get_next_word(fd, next_word);

    if (next_status == 1) {
      // Fine file raggiunta - current_word era l'ultima parola
      free_word(word);
      break;
    }

    if (next_status == -1) {
      // Errore nella lettura della parola successiva
      free_word(word);
      break;
    }

    // Verifica che next_word sia valida
    if (next_word[0] == 0) {
      // Parola successiva vuota, libera current_word e continua
      free_word(word);
      // Sposta current_word per il prossimo ciclo
      memset(current_word, 0, sizeof(current_word));
      move_cursor_to_next_word(fd);
      status = f_get_next_word(fd, current_word);
      if (status != 0)
        break;
      continue;
    }

    // Crea word_t per next_word
    word_t *next_word_obj = create_word(next_word);
    if (next_word_obj == NULL) {
      fprintf(stderr, "Failed to create next word from next_word\n");
      free_word(word);
      // Copia next_word in current_word per il prossimo ciclo
      memcpy(current_word, next_word, sizeof(current_word));
      continue;
    }

    // Crea e inserisci l'item nella hash table
    ht_item *item = word_ht_item_create(word, next_word_obj);
    if (item == NULL) {
      fprintf(stderr, "Failed to create hash table item\n");
      free_word(word);
      free_word(next_word_obj);
      // Copia next_word in current_word per il prossimo ciclo
      memcpy(current_word, next_word, sizeof(current_word));
      continue;
    }

    ht_insert(table, item);

    // Prepara per il prossimo ciclo: next_word diventa current_word
    memcpy(current_word, next_word, sizeof(current_word));
    memset(next_word, 0, sizeof(next_word));
  }

  return table;
}
int f_get_next_word(int fd, int *word_buffer) {
  if (fd < 0) {
    fprintf(stderr, "Invalid file descriptor\n");
    return -1;
  }

  int bytes_read = 0;
  int index = 0;
  int cp;

  // Prima salta spazi e punteggiatura ignorabile
  while ((cp = utf8_getchar_fd(fd, &bytes_read)) != EOF) {
    if (cp == UTF8_ERROR) {
      fprintf(stderr, "Error reading UTF-8 character\n");
      return -1;
    }

    // Se troviamo spazi o newline, continuiamo
    if (cp == ' ' || cp == '\n') {
      continue;
    }

    // Se troviamo punteggiatura da ignorare, continuiamo
    if (is_punctuation(cp)) {
      continue;
    }

    // Se troviamo terminatori di frase, li restituiamo come parole singole
    if (cp == '.' || cp == '?' || cp == '!') {
      word_buffer[index++] = cp;
      word_buffer[index] = 0;
      return 0;
    }

    // Primo carattere valido di una parola normale
    word_buffer[index++] = cp;
    break;
  }

  // Se abbiamo raggiunto EOF senza trovare caratteri
  if (cp == EOF) {
    if (index == 0) {
      return 1; // EOF
    }
  }

  // Continua a leggere il resto della parola
  while ((cp = utf8_getchar_fd(fd, &bytes_read)) != EOF) {
    if (cp == UTF8_ERROR) {
      fprintf(stderr, "Error reading UTF-8 character\n");
      return -1;
    }

    // Se incontriamo uno spazio, fine parola
    if (cp == ' ' || cp == '\n') {
      break;
    }

    // Se incontriamo terminatori di frase, torniamo indietro e finiamo la
    // parola
    if (cp == '.' || cp == '?' || cp == '!') {
      lseek(fd, -bytes_read, SEEK_CUR);
      break;
    }

    // Se incontriamo un apostrofo, torniamo indietro e finiamo la parola
    if (cp == '\'') {
      word_buffer[index++] = cp; // Aggiungi l'apostrofo alla parola
      word_buffer[index] = 0;    // Null-terminate
      return 0;                  // Fine della parola
    }

    // Se incontriamo punteggiatura da ignorare, la saltiamo
    if (is_punctuation(cp)) {
      continue;
    }

    // Controlla lunghezza massima
    if (index >= MAX_WORD_LENGTH - 1) {
      fprintf(stderr, "Word exceeds maximum length\n");
      return -1;
    }

    // Aggiungi carattere alla parola
    word_buffer[index++] = cp;
  }

  word_buffer[index] = 0; // Null-terminate
  return 0;
}

void move_cursor_to_next_word(int fd) {
  int cp;
  int bytes_read = 0;

  while ((cp = utf8_getchar_fd(fd, &bytes_read)) != EOF) {
    // Errore nella lettura UTF-8 → avanza di 1 byte e continua
    if (cp == UTF8_ERROR) {
      lseek(fd, 1, SEEK_CUR);
      continue;
    }

    // Spazi o newline → continua avanzando
    if (cp == ' ' || cp == '\n') {
      continue;
    }

    // Punteggiatura da scartare → continua avanzando
    if (is_punctuation(cp)) {
      continue;
    }

    // Primo carattere valido → torna indietro e lascia che lo legga
    // f_get_next_word
    lseek(fd, -bytes_read, SEEK_CUR);
    return;
  }
}

static int is_punctuation(const int cp) {
  return (cp == ',' || cp == ';' || cp == ':' || cp == '-' || cp == '\"');
}
