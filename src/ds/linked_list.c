#include "linked_list.h"
#include "utils.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// constructor for the linked list
linked_list_t *create_linked_list() {
  linked_list_t *list = dmalloc(sizeof(linked_list_t));
  list->head = NULL;
  list->tail = NULL;
  return list;
}

void add_to_list(linked_list_t *list, void *data) {
  if (!list) {
    fprintf(stderr, "List is NULL\n");
    return;
  }

  ll_item_t *node = dmalloc(sizeof(ll_item_t));
  node->data = data;
  node->next = NULL;

  if (!list->head) {
    list->head = list->tail = node;
    return;
  }

  list->tail->next = node;
  list->tail = node;
}

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
      free(current);
      return;
    }
    previous = current;
    current = current->next;
  }

  fprintf(stderr, "Data not found in the list\n");
}

void free_linked_list(linked_list_t *list) {
  if (!list) {
    fprintf(stderr, "list is null\n");
    return;
  }

  ll_item_t *current = list->head;
  while (current) {
    ll_item_t *next = current->next;
    free(current->data);
    free(current);
    current = next;
  }
  free(list);
}

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

void **linked_list_to_array(const linked_list_t *list) {
  if (!list || !list->head) {
    fprintf(stderr, "List is empty or NULL\n");
    return NULL;
  }

  size_t size = get_list_size(list);
  void **array = dmalloc(size * sizeof(void *));

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
