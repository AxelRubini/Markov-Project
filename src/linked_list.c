#include "../include/linked_list.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

//constructor for the linked list
linked_list_t*create_linked_list(){
    linked_list_t*list = dmalloc(sizeof(linked_list_t));
    list->head = NULL;
    list->tail = NULL;
    return list;
}



void add_to_list(linked_list_t*list ,void *data) {
    if (list == NULL) {
        fprintf(stderr, "List is NULL\n");
        return;
    }
    ll_item_t*new_item = dmalloc(sizeof(ll_item_t));
    new_item -> data = data;
    new_item -> next = NULL;

    if (list->head == NULL){
        list->head = new_item;
        list->tail = new_item;

    } else {
        list->tail->next = new_item;
        list->tail = new_item;
    }
    list->tail->next = NULL;

    return;
}

void remove_from_list(linked_list_t*list, void *data){
    if (list == NULL || list->head == NULL) {
        fprintf(stderr, "List is empty or NULL\n");
        return;
    }

    ll_item_t*current = list->head;
    ll_item_t*previous = NULL;

    while (current != NULL) {
        if (current->data == data) {
            if (previous == NULL) {
                // Removing the head
                list->head = current->next;
                if (list->head == NULL) {
                    list->tail = NULL; // List is now empty
                }
            } else {
                previous->next = current->next;
                if (current->next == NULL) {
                    list->tail = previous; // Update tail if we removed the last item
                }
            }
            free(current);
            return;
        }
        previous = current;
        current = current->next;
    }
    fprintf(stderr, "Data not found in the list\n");



}

void free_linked_list(linked_list_t*list){
    if (list == NULL) {
        fprintf(stderr, "list is null\n");
        return;
    }

    ll_item_t*current = list->head;
    while (current != NULL) {
        ll_item_t*next = current->next;
        free(current->data);
        free(current);
        current = next;
    }
    free(list);
    list = NULL;
    return;
}

int get_list_size(linked_list_t*list) {
    if (list == NULL) {
        fprintf(stderr, "List is NULL\n");
        return 0;
    }

    int size = 0;
    ll_item_t*current = list->head;
    while (current != NULL) {
        size++;
        current = current->next;
    }
    return size;
}

void* linked_list_to_array(linked_list_t*list){
    if (list == NULL || list->head == NULL){
        fprintf(stderr, "List is empty or NULL\n");
        return NULL;
    }
    int size = get_list_size(list);
    void **array = dmalloc(size * sizeof(void *));

    ll_item_t*current = list->head;
    for (int i = 0; i < size; i++) {
        if (current == NULL) {
            fprintf(stderr, "Unexpected end of list\n");
            free(array);
            return NULL;
        }
        array[i] = current->data;
        current = current->next;
    }
    return array;
}
