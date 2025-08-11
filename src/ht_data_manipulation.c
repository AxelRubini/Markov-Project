#include "../include/ht_data_manipulation.h"

void create_csv_line(ht_item *item, char *buffer) {
  if (!item || !buffer) {
    if (!buffer) {
      fprintf(stderr, "Invalid buffer pointer\n");
    }
    return;
  }
  
  int offset = 0;
  const word_t *key = (const word_t *)item->key;
  const int *word = key->word;
  const linked_list_t *next_words = (const linked_list_t *)item->value;
  
  // Controllo di sicurezza
  if (!key || !word || !next_words) {
    fprintf(stderr, "Invalid item structure\n");
    return;
  }
  
  // Scrivi la parola chiave
  for (int i = 0; word[i] != '\0'; i++) {
    utf8_append_char(word[i], buffer, &offset);
  }
  
  // Se non ci sono parole successive, termina solo con la chiave
  if (!next_words->head) {
    buffer[offset++] = '\n';
    buffer[offset] = '\0';
    return;
  }
  
  buffer[offset++] = ','; // Virgola dopo la parola chiave
  
  // Processa tutti gli elementi della lista
  ll_item_t *current = next_words->head;
  while (current) {
    const word_t *next_word = (const word_t *)current->data;
    
    if (!next_word || !next_word->word) {
      fprintf(stderr, "Invalid next_word in list\n");
      current = current->next;
      continue;
    }
    
    const int *next_word_codepoints = next_word->word;
    
    // Scrivi la parola successiva
    for (int j = 0; next_word_codepoints[j] != '\0'; j++) {
      utf8_append_char(next_word_codepoints[j], buffer, &offset);
    }
    
    buffer[offset++] = ','; // Virgola dopo la parola
    
    // Calcola e scrivi la probabilità
    double probability = next_word_probability(&key->occurrences, &next_word->occurrences);
    if (probability < 0.0 || probability > 1.0) {
      fprintf(stderr, "Invalid probability value: %f\n", probability);
      probability = 0.0; // Imposta a zero se la probabilità è fuori i limiti
    } 
    char prob_str[FLOATING_POINT_CHAR_SPACE];
    snprintf(prob_str, FLOATING_POINT_CHAR_SPACE, "%.6f", probability);
    
    for (int j = 0; prob_str[j] != '\0'; j++) {
      utf8_append_char(prob_str[j], buffer, &offset);
    }
    
    current = current->next;
    
    // Se non è l'ultimo elemento, aggiungi virgola
    if (current) {
      buffer[offset++] = ',';
    }
  }
  
  buffer[offset++] = '\n'; // Newline alla fine
  buffer[offset] = '\0';   // Null-terminate
}
/*
 * This function calculates the size of the buffer needed to store a
 * line for the CSV file.
 * It consider the fact that each word can be up to MAX_WORD_LENGTH but
 * since the words are UTF-8 encoded, we need to consider the maximum number of
 * bytes
 * NOTE: This function can be optimized to be more efficient and precise but for
 * time being it is a good approximation.
 */
int calculate_buffer_size(ht_item *item) {
  if (!item) {
    return 0;
  }
  int size = MAX_WORD_LENGTH * 4; // Assuming a maximum word length
  const linked_list_t *next_words = (linked_list_t *)item->value;
  int list_size = get_list_size(next_words);
  size += ((list_size * MAX_WORD_LENGTH * 4) +
           (list_size * FLOATING_POINT_CHAR_SPACE) +
           2 * list_size); // Add size for next words

  return size;
}
