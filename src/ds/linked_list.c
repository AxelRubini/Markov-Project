#include "linked_list.h"
#include "utils.h"
#include <stdio.h>
#include <stdlib.h>

/* Create an empty linked list.
 * Allocates list structure; release with free_linked_list().
 */
linked_list_t *create_linked_list() {
  linked_list_t *list = dmalloc(sizeof(linked_list_t)); // allocate list
  list->head = NULL;
  list->tail = NULL;
  return list;
}

/* Append data at the tail.
 * list: must be non-NULL.
 * data: element pointer; not copied or freed.
 */
void add_to_list(linked_list_t *list, void *data) {
  if (!list) { // precondition check
    fprintf(stderr, "List is NULL\n");
    return;
  }

  ll_item_t *node = dmalloc(sizeof(ll_item_t)); // allocate node
  node->data = data;
  node->next = NULL;

  if (!list->head) { // first element
    list->head = list->tail = node;
    return;
  }

  list->tail->next = node;
  list->tail = node;
}

/* Remove first node matching data pointer.
 * Only the node is freed; caller manages stored data.
 */
void remove_from_list(linked_list_t *list, void *data) {
  if (!list || !list->head) {
    fprintf(stderr, "List is empty or NULL\n");
    return;
  }

  ll_item_t *current = list->head;
  ll_item_t *previous = NULL;

  while (current) {
    if (current->data == data) {
      if (previous) {
        previous->next = current->next;
        if (!current->next)
          list->tail = previous;
      } else {
        list->head = current->next;
        if (!list->head)
          list->tail = NULL;
      }
      free(current); // free node memory
      return;
    }
    previous = current;
    current = current->next;
  }

  fprintf(stderr, "Data not found in the list\n");
}

/* Free list nodes and optionally their data.
 * free_func releases stored elements when provided.
 */
void free_linked_list(linked_list_t *list, void (*free_func)(void *)) {
  if (!list) {
    fprintf(stderr, "list is null\n");
    return;
  }

  ll_item_t *current = list->head;
  while (current) {
    ll_item_t *next = current->next;
    if (free_func) {
      free_func(current->data); // caller-provided cleanup
    }
    free(current);
    current = next;
  }
  free(list);
}

/* Count elements in list.
 * Returns 0 if list is NULL.
 */
size_t get_list_size(const linked_list_t *list) {
  if (!list) {
    fprintf(stderr, "List is NULL\n");
    return 0;
  }

  size_t size = 0;
  ll_item_t *current = list->head;
  while (current) {
    size++;
    current = current->next;
  }
  return size;
}

/* Convert list to newly allocated array.
 * Returns NULL on error or empty list; caller frees array.
 */
void **linked_list_to_array(const linked_list_t *list) {
  if (!list || !list->head) {
    fprintf(stderr, "List is empty or NULL\n");
    return NULL;
  }

  size_t size = get_list_size(list);
  void **array = dmalloc(size * sizeof(void *)); // allocate array

  ll_item_t *current = list->head;
  for (size_t i = 0; i < size; i++) {
    if (!current) {
      fprintf(stderr, "Unexpected end of list\n");
      free(array);
      return NULL;
    }
    array[i] = current->data;
    current = current->next;
  }
  return array;
}
