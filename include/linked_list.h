#ifndef LINKED_LIST_H
#define LINKED_LIST_H

typedef struct ll_item {
    void *data;
    struct ll_item *next;
} ll_item;

typedef struct {
    ll_item *head;
    ll_item *tail;
} linked_list;

typedef struct {
    ll_item *current;
} linked_list_iterator;

linked_list *create_linked_list();
void add_to_list(linked_list *list, void *data);
void remove_from_list(linked_list *list, void *data);
void free_linked_list(linked_list *list);
int get_list_size(linked_list *list);
void* linked_list_to_array(linked_list *list);
#endif