/* =====================================================
 * test_ds.c  —  Unit tests for custom data structures
 * =====================================================
 * This file verifies the correctness of:
 *   • linked list (linked_list.[ch])
 *   • generic separate-chaining hash table (hash_table.[ch])
 *   • word follower table based on the hash table (word.[ch])
 *
 * Build:  gcc -Wall -Wextra -pedantic -std=c17 *.c -o tests && ./tests
 * NB:  All malloc calls must be replaced by the project-provided dmalloc()!
 * -----------------------------------------------------*/

#include "../include/hash_table.h"
#include "../include/ht_item.h"
#include "../include/linked_list.h"
#include "../include/utils.h"
#include "../include/word.h"

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

/* -----------------------------------------------------
 * Helpers for <char*, int> table used in basic tests
 * -----------------------------------------------------*/
static unsigned int str_hash(const void *key, int size) {
  const unsigned char *str = (const unsigned char *)key;
  unsigned long hash = 5381;
  int c;
  while ((c = *str++)) {
    hash = ((hash << 5) + hash) + c; /* hash * 33 + c */
  }
  return (unsigned int)(hash % size);
}

static int str_cmp(const void *a, const void *b) {
  return strcmp((const char *)a, (const char *)b);
}

/* -----------------------------------------------------
 * Linked-list tests (unchanged)
 * -----------------------------------------------------*/
static void test_linked_list(void) {
  linked_list_t *list = create_linked_list();

  int *a = dmalloc(sizeof *a);
  *a = 1;
  int *b = dmalloc(sizeof *b);
  *b = 2;
  int *c = dmalloc(sizeof *c);
  *c = 3;

  add_to_list(list, a);
  add_to_list(list, b);
  add_to_list(list, c);

  assert(get_list_size(list) == 3);

  int **arr = (int **)linked_list_to_array(list);
  assert(arr[0] == a && arr[1] == b && arr[2] == c);
  free(arr);

  remove_from_list(list, b); /* detach node – int still ours */
  free(b);                   /* free it explicitly          */
  assert(get_list_size(list) == 2);

  free_linked_list(list); /* frees nodes + (a and c)     */
}

/* -----------------------------------------------------
 * Hash-table tests with <char*,int> pairs
 * -----------------------------------------------------*/
static void test_ht_table_basic(void) {
  hash_table_t *ht = create_hash_table(53, str_hash, str_cmp);

  char keybuf[16];
  for (int i = 0; i < 100; i++) {
    snprintf(keybuf, sizeof keybuf, "key%d", i);
    char *key = dmalloc(strlen(keybuf) + 1);
    strcpy(key, keybuf);

    int *val = dmalloc(sizeof *val);
    *val = i;

    /* wrap the pair into a generic ht_item and insert */
    ht_insert(ht, default_create_ht_item(key, val));
  }

  assert(ht_get_count(ht) == 100);

  ht_item *f = ht_search(ht, "key42");
  int *tmp = f ? (int *)f->value : NULL;
  assert(tmp && *tmp == 42);

  ht_remove(ht, "key42");
  assert(ht_search(ht, "key42") == NULL);

  free_hash_table(ht);
}

/* -----------------------------------------------------
 * Direct unit tests for ht_item update_value –
 *   1) primitive types via default_update_value
 *   2) word follower logic via update_ht_item_value
 * -----------------------------------------------------*/
static void test_ht_item_update_value(void) {
  /* --- 1. Primitive <char*,int> pair ---------------------------------- */
  char *pkey = dmalloc(4); /* "foo" */
  strcpy(pkey, "foo");

  int *v1 = dmalloc(sizeof *v1);
  *v1 = 10;
  ht_item *pit = default_create_ht_item(pkey, v1);

  /* replace value with v2 using default_update_value */
  int *v2 = dmalloc(sizeof *v2);
  *v2 = 20;
  pit->update_value(pit, v2); /* default_update_value just swaps the pointer */
  assert(*(int *)pit->value == 20);

  /* cleanup primitive case */
  default_free_ht_item(pit); /* frees the wrapper only */
  free(v1);
  free(v2);
  free(pkey);
  printf("Primitive update_value test passed.\n");

  /* --- 2. Word-based follower list ------------------------------------ */
  int w_key_arr[] = {'o', 'g', 'g', 'i', '\0'};      /* key = "oggi"  */
  int w_follow1[] = {'t', 'e', 'm', 'p', 'o', '\0'}; /* follower 1 */
  int w_follow2[] = {'c', 'a', 'l', 'd', 'o', '\0'}; /* follower 2 */

  word_t *wkey = create_word(w_key_arr);
  word_t *wf1 = create_word(w_follow1);

  ht_item *wit = word_ht_item_create(wkey, wf1);

/* same follower again → occurrences must grow to 2 */
  wit->update_value(wit, w_follow1);
/* new follower → lista deve contenere due nodi */
  wit->update_value(wit, w_follow2);

  linked_list_t *followers = (linked_list_t *)wit->value;
  assert(get_list_size(followers) == 2);

  /* verify occurrences count */
  word_t **arr = (word_t **)linked_list_to_array(followers);
  int found_tempo = 0, found_caldo = 0;
  for (int i = 0; i < 2; ++i) {
    if (word_str_cmp(arr[i]->word, w_follow1) == 0) {
      assert(arr[i]->occurrences == 2);
      found_tempo = 1;
    } else if (word_str_cmp(arr[i]->word, w_follow2) == 0) {
      assert(arr[i]->occurrences == 1);
      found_caldo = 1;
    }
  }
  assert(found_tempo && found_caldo);

  /* cleanup word case */
   wit->free_item(wit); /* frees key + follower list */

}

/* -----------------------------------------------------
 * Word-follower hash-table tests (unchanged)
 * -----------------------------------------------------*/
static void test_word_followers_hash_table(void) {
  hash_table_t *ht = create_hash_table(101, word_hash, word_hashtable_keycmp);

  /* ---- parole di prova ---- */
  int w1[] = {'o', 'g', 'g', 'i', '\0'};
  int w2[] = {'t', 'e', 'm', 'p', 'o', '\0'};
  int w3[] = {'i', 'n', 'c', 'e', 'r', 't', 'o', '\0'};

  word_t *k_oggi = create_word(w1);
  word_t *v_tempo = create_word(w2);
  word_t *v_incerto = create_word(w3);

  /* ---- inserimenti / aggiornamenti ---- */
  ht_insert(ht, word_ht_item_create(k_oggi, v_incerto));
  ht_insert(ht, word_ht_item_create(k_oggi, v_tempo));
  ht_insert(ht, word_ht_item_create(k_oggi, v_incerto));

  /* ---- verifiche ---- */
  ht_item *res = ht_search(ht, k_oggi);
  linked_list_t *followers = res ? (linked_list_t *)res->value : NULL;
  assert(followers != NULL);
  assert(get_list_size(followers) == 2);

  word_t **arr = (word_t **)linked_list_to_array(followers);
  int found_tempo = 0, found_inc = 0;

  for (int i = 0; i < 2; ++i) {
    if (wordcmp(arr[i], v_tempo) == 0) {
      assert(arr[i]->occurrences == 2);
      found_tempo = 1;
    } else if (wordcmp(arr[i], v_incerto) == 0) {
      assert(arr[i]->occurrences == 2);
      found_inc = 1;
    }
  }
  assert(found_tempo && found_inc);

  /* cleanup */
  free(arr);
  free_hash_table(ht);
}

/* -----------------------------------------------------
 * Main: run the full test suite
 * -----------------------------------------------------*/
int main(void) {
  printf("Running tests...\n");

  test_linked_list();
  printf("Linked list tests passed.\n");

  test_ht_table_basic();
  printf("Hash table basic tests passed.\n");

  test_ht_item_update_value();
  printf("Hash table item update value tests passed.\n");

  test_word_followers_hash_table();
  printf("Word followers hash table tests passed.\n");

  printf("All tests passed successfully!\n");
  return 0;
}
