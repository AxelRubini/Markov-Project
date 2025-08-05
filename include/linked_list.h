#ifndef LINKED_LIST_H
#define LINKED_LIST_H

typedef struct ll_item {
  void *data;
  struct ll_item *next;
} ll_item_t;

typedef struct {
  ll_item_t *head;
  ll_item_t *tail;
} linked_list_t;

typedef struct {
  ll_item_t *current;
} linked_list_iterator_t;

linked_list_t*create_linked_list();
void add_to_list(linked_list_t*list, void *data);
void remove_from_list(linked_list_t*list, void *data);
void free_linked_list(linked_list_t*list);
int get_list_size(linked_list_t*list);
void *linked_list_to_array(linked_list_t*list);
#endif
