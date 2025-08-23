#ifndef LINKED_LIST_H
#define LINKED_LIST_H

#include <stddef.h>

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

/**
 * @brief Create an empty linked list.
 *
 * @return Newly allocated list or NULL on error.
 */
linked_list_t *create_linked_list();

/**
 * @brief Append data to the end of a list.
 *
 * @param list List to modify.
 * @param data Element pointer.
 */
void add_to_list(linked_list_t *list, void *data);

/**
 * @brief Remove the first occurrence of data from a list.
 *
 * @param list List to modify.
 * @param data Element to remove.
 */
void remove_from_list(linked_list_t *list, void *data);

/**
 * @brief Free a list and its nodes.
 *
 * Each element stored in the list is freed using \p free_func if provided.
 * This allows the caller to specify how to release the memory pointed by the
 * data field of each node, avoiding both leaks and invalid frees.
 *
 * @param list List to free.
 * @param free_func Function used to free the stored data (may be NULL).
 */
void free_linked_list(linked_list_t *list, void (*free_func)(void *));

/**
 * @brief Get the number of elements in a list.
 *
 * @param list List to inspect.
 * @return Element count.
 */
size_t get_list_size(const linked_list_t *list);

/**
 * @brief Convert a list to an array of pointers.
 *
 * @param list Source list.
 * @return Newly allocated array of elements.
 */
void **linked_list_to_array(const linked_list_t *list);
#endif
