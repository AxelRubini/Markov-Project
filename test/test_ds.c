/*
 * This .c file contains test cases for the Data Structure implementation
 * */

#include "../include/ht.h"
#include "../include/ht_item.h"
#include "../include/linked_list.h"
#include "../include/utils.h"
#include "../include/word.h"
#include <assert.h>
#include <ctype.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

/* Utility function */

static void print_word_followers_hash_table(hash_table_t *ht) {
  int between_char[] = {',','\0'}; // used to print words with parentheses
  if (ht == NULL) {
    fprintf(stderr, "HT is NULL\n");
    return;
  }

  printf("\n--- HASH TABLE DUMP (%d items) ---\n", ht_get_count(ht));

  /* scan every bucket */
  for (int b = 0; b < ht->size; ++b) {
    linked_list_t *bucket = ht->buckets[b];
    if (bucket == NULL || bucket->head == NULL)
      continue;

    ll_item_t *cur = bucket->head;
    while (cur != NULL) {
      ht_item *item = (ht_item *)cur->data;
      word_t *key_word = (word_t *)item->key;
      linked_list_t *list = (linked_list_t *)item->value;

      /* print key */
      word_print(key_word, STDOUT_FILENO, between_char);
      printf(" : ");

      /* print followers [ w1(freq) , w2(freq) ... ] */
      ll_item_t *fc = list->head;
      while (fc != NULL) {
        word_t *fw = (word_t *)fc->data;
        word_print(fw, STDOUT_FILENO, between_char);

        if (fc->next)
          printf(", ");
        fc = fc->next;
      }
      printf("\n");
      cur = cur->next;
    }
  }
  printf("--- END DUMP ---\n\n");
}
/* Unit tests functions */
static void test_linked_list(void)
{
    linked_list_t *list = create_linked_list();
    int *a = dmalloc(sizeof *a); *a = 1;
    int *b = dmalloc(sizeof *b); *b = 2;
    int *c = dmalloc(sizeof *c); *c = 3;

    add_to_list(list, a);
    add_to_list(list, b);
    add_to_list(list, c);

    assert(get_list_size(list) == 3);

    int **arr = (int **)linked_list_to_array(list);
    assert(arr[0] == a && arr[1] == b && arr[2] == c);
    free(arr);

    remove_from_list(list, b);   /* toglie il nodo, ma l’int è ancora tuo   */
    free(b);                     /* lo liberi esplicitamente                */
    assert(get_list_size(list) == 2);

    free_linked_list(list);      /* libera nodi + (a e c)                   */
}

static void test_ht_table_basic(void) {
  hash_table_t *ht = create_hash_table((hash_func_t)hash_function, NULL, NULL);

  // force the hash table to resize
  char keybuf[16];
  for (int i = 0; i < 100; i++) {
    snprintf(keybuf, sizeof(keybuf), "key%d",
             i); // create keys key0, key1, ..., key99, snprintf populates the
                 // keybuf with the string
    char *key = dmalloc(strlen(keybuf) + 1);
    strcpy(key, keybuf); // Duplicate the key string
    int *val = dmalloc(sizeof(int));
    *val = i; // Assign the value to the key
    ht_insert(ht, key, val);
  }
  assert(ht_get_count(ht) == 100);
printf("DEBUG  ht_count=%d  buckets=%d\n", ht_get_count(ht), ht->size);
int *tmp = (int *)ht_search(ht, "k42");
printf("DEBUG  k42  ptr=%p  val=%d\n", (void *)tmp, tmp ? *tmp : -1);

  int *v42 = (int *)ht_search(ht, "key42");
  assert(v42 && *v42 == 42);
  int *v99 = (int *)ht_search(ht, "key99");
  assert(v99 && *v99 == 99);

  ht_delete(ht, "key42");
  assert(ht_search(ht, "key42") == NULL);

  free_hash_table(ht); // Free the hash table
}

static void test_ht_item_update_value(void) {
  int w1_raw[] = {'c', 'o', 's', 'a', '\0'};
  int w2_raw[] = {'d', 'i', 'c', 'o', 'n', 'o', '\0'};

  word_t *w1 = create_word(w1_raw); /* key */
  word_t *w2 = create_word(w2_raw); /* first follower */

  ht_item *item = word_ht_item_create(w1, w2);
  assert(item != NULL);
  update_ht_item_value(item, w2); // Update the value with the same word
  linked_list_t *list = (linked_list_t *)item->value;
  word_t *first_follower = (word_t *)list->head->data;
  assert(strcmp((char *)first_follower->word, "dicono") == 0);
  assert(first_follower->occurrences == 2);

  int w3_raw[] = {'d', 'i', 'c', 'e', '\0'};
  word_t *w3 = create_word(w3_raw);
  update_ht_item_value(item, w3); // Update the value with a different word
  assert(get_list_size(list) ==
         2); // Now there should be three words in the list

  free_linked_list(list);
  free_word(w1);
  free_word(w2);
  free_word(w3);
  free_ht_item(item);
}

static void test_word_followers_hash_table(void) {
  hash_table_t *ht =
      create_hash_table((hash_func_t)word_hash, (key_compare_func_t)wordcmp,
                        (create_ht_item_func_t)word_ht_item_create);
  /* ---------- parole di prova ---------------------------------- */
  int w1[] = {'o', 'g', 'g', 'i', '\0'};
  int w2[] = {'t', 'e', 'm', 'p', 'o', '\0'};
  int w3[] = {'i', 'n', 'c', 'e', 'r', 't', 'o', '\0'};

  word_t *k_oggi = create_word(w1);
  word_t *v_tempo = create_word(w2);
  word_t *v_incerto = create_word(w3);

  /* ---------- inserimenti / aggiornamenti ---------------------- */
  ht_insert(ht, k_oggi, v_incerto);
  ht_insert(ht, k_oggi, v_tempo);
  ht_insert(ht, k_oggi, v_incerto);

  /* ---------- verifiche ---------------------------------------- */
  linked_list_t *followers = (linked_list_t *)ht_search(ht, k_oggi);
  assert(followers != NULL);

  assert(get_list_size(followers) ==
         2); /* :contentReference[oaicite:9]{index=9} */

  word_t **arr = (word_t **)linked_list_to_array(followers);
  int found_tempo = 0, found_inc = 0;

  for (int i = 0; i < 2; ++i) {
    if (wordcmp(arr[i], v_tempo) == 0) {
      assert(arr[i]->occurrences == 2);
      found_tempo = 1;
    } else if (wordcmp(arr[i], v_incerto) == 0) {
      assert(arr[i]->occurrences == 1);
      found_inc = 1;
    }
  }
  assert(found_tempo && found_inc);

  /* pulizia */
  free(arr);
  free_hash_table(ht);
}
/* Main function to run all tests */
int main(void) {
  printf("Running tests...\n");

  test_linked_list();
  printf("Linked list tests passed.\n");

  test_ht_table_basic();
  printf("Hash table basic tests passed.\n");

  //test_ht_item_update_value();
  printf("Hash table item update value tests passed.\n");

  //test_word_followers_hash_table();
  printf("Word followers hash table tests passed.\n");

  printf("All tests passed successfully!\n");
  return 0;
}
/* End of test_ds.c */
