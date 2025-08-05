#include "../include/word.h"
#include "../include/utf8_tools.h"
#include "../include/utils.h"
#include "../include/ht_item.h"
#include "../include/linked_list.h"
#include <stdlib.h>
#include <stdio.h>


word_t *create_word(int *word){
    if (word == NULL){
        fprintf(stderr, "Word is NULL\n");
        return NULL;
    }

    word_t *new_word = dmalloc(sizeof(word_t));


    new_word->word = utf8_word_to_lower(word, MAX_WORD_LENGTH);
    new_word->occurrences = 1;

    return new_word;
}

ht_item *word_ht_item_create(word_t *key, word_t *value) {
    if (key == NULL || value == NULL) {
        fprintf(stderr, "Key or value is NULL\n");
        return NULL;
    }

    ht_item *item = dmalloc(sizeof(ht_item));
    item->key = key;
    item->update_value = update_ht_item_value; // Set the update function
    item->value = create_linked_list(); // Initialize value to the word_t pointer
    add_to_list((linked_list_t *)item->value, value); // Add the value to the linked list    

    return item;
} 

void update_word_occurrences(word_t *word) {
    if (word == NULL) {
        fprintf(stderr, "Word is NULL\n");
        return;
    }

    word->occurrences++;
}

int get_word_occurrences(const word_t *word) {
    if (word == NULL) {
        fprintf(stderr, "Word is NULL\n");
        return 0;
    }

    return word->occurrences;
}

int wordcmp(const word_t *word1, const word_t *word2) {
    if (word1 == NULL || word2 == NULL) {
        fprintf(stderr, "One or both words are NULL\n");
        return 0;
    }

    int i = 0;
    while (word1->word[i] != '\0' && word2->word[i] != '\0') {
        if (word1->word[i] != word2->word[i]) {
            return word1->word[i] - word2->word[i];
        }
        i++;
    }
    return word1->word[i] - word2->word[i];
}

void update_ht_item_value( const void *item,const  void *new_value) {
    if (item == NULL || new_value == NULL) {
        fprintf(stderr, "Item or new value is NULL\n");
        return;
    }
    ht_item *htItem = (ht_item *)item;
    linked_list_t *list = (linked_list_t *)htItem->value;
    ll_item_t *current = list->head;
    while (current != NULL) {
        word_t *word = (word_t *)current->data;
        if (wordcmp(word, (int *)new_value) == 0) {
            // If the word already exists, update its occurrences
            update_word_occurrences(word);
            return;
        }
        current = current->next;
        }
    // If the word does not exist, add it to the linked list
    word_t *new_word = create_word((int *)new_value);
    add_to_list(list, new_word);    
}

void free_word(word_t *word) {
    if (word == NULL) {
        return;
    }
    if (word->word != NULL) {
        free((void *)word->word); // Free the word array
    }
    

    free(word);
}



