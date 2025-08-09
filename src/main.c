
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_WORD_LENGTH                                                        \
  31 // Maximum length of a word in code points with null terminato

int main(void) {
  char *str = "oggi e un bel giorno, domani e un giorno soleggiato";

  char current_word[MAX_WORD_LENGTH] = {0}; // buffer for the current word
  char next_word[MAX_WORD_LENGTH] = {0};    // buffer for the next word
  char reading_next = 0;
  size_t c_idx = 0; // current index in the string
  size_t n_idx = 0; // next index in the string
  for (int i = 0; str[i] != '\0'; i++) {
    printf("char: %c\n", str[i]);
    if (reading_next != 0) {
      if (str[i] == ' ') {
        reading_next = 1;
        current_word[c_idx] = '\0'; // terminate the current word
        c_idx = 0;
      } else {
        current_word[c_idx] = str[i];
        c_idx++;

      }
    } else {
      if (str[i] == ' ') {
        reading_next = 0;
        printf("la parola corrente e: %s\n la prossima parola e: %s\n",
               current_word, next_word);
        // free(current_word);
        // free(next_word);
        next_word[n_idx] = '\0'; // terminate the next word
        n_idx = 0; // reset next index
        strcpy(current_word, next_word); // copy next to current
        // reset next word buffer
        memset(next_word, 0, sizeof(next_word));
      } else {
        next_word[n_idx] = str[i];
        n_idx++;
      }
    }
  }
}
/*
int main() {
  char stringa[] = "oggi e un bel giorno,domani e un giorno soleggiato";
  int i = 0, start1, start2;

  while (stringa[i] != '\0') {

    while (stringa[i] == ' ' || stringa[i] == ',')
      i++;
    if (stringa[i] == '\0')
      break;
    start1 = i;

    // vai alla fine della prima parola
    while (stringa[i] != '\0' && stringa[i] != ' ' && stringa[i] != ',')
      i++;

    // trova inizio seconda parola
    while (stringa[i] == ' ' || stringa[i] == ',')
      i++;
    if (stringa[i] == '\0')
      break;
    start2 = i;

    // stampa prima parola
    int j = start1;
    while (stringa[j] != '\0' && stringa[j] != ' ' && stringa[j] != ',') {
      putchar(stringa[j]);
      j++;
    }

    putchar(' '); // separatore

    // stampa seconda parola
    j = start2;
    while (stringa[j] != '\0' && stringa[j] != ' ' && stringa[j] != ',') {
      putchar(stringa[j]);
      j++;
    }

    putchar('\n');

    // riposiziona i alla seconda parola per continuare
    i = start2;
  }

  return 0;
}*/
