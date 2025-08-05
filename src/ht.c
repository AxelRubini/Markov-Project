#include "../include/ht.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>


hash_table_t *create_hash_table(unsigned int (*hash_func)(void *, int)) {
    hash_table_t *ht = malloc(sizeof(hash_table_t));
    if (ht == NULL) {
        fprintf(stderr, "Memory allocation failed for hash table\n");
        return NULL;
    }
    
    ht->size = START_SIZE;
    ht->count = 0;
    
    
    // Alloca l'array di buckets
    ht->buckets = malloc(sizeof(linked_list_t*) * ht->size);
    if (ht->buckets == NULL) {
        fprintf(stderr, "Memory allocation failed for buckets\n");
        free(ht);
        return NULL;
    }
    
    // Inizializza ogni bucket con una linked list vuota
    for (int i = 0; i < ht->size; i++) {
        ht->buckets[i] = create_linked_list();
        if (ht->buckets[i] == NULL) {
            // Cleanup in caso di errore
            for (int j = 0; j < i; j++) {
                free_linked_list(ht->buckets[j]);
            }
            free(ht->buckets);
            free(ht);
            return NULL;
        }
    }
    
    return ht;
}

double ht_load_factor(hash_table_t *ht) {
    if (ht == NULL || ht->size == 0) return 0.0;
    return (double)ht->count / (double)ht->size;
}

void ht_resize(hash_table_t *ht) {
    if (ht == NULL) return;
    
    // Salva la vecchia configurazione
    linked_list_t **old_buckets = ht->buckets;
    int old_size = ht->size;
    
    // Raddoppia la dimensione
    ht->size = old_size * 2;
    ht->count = 0; // Riazzerato, verrà incrementato durante il reinserimento
    
    // Alloca nuovi buckets
    ht->buckets = malloc(sizeof(linked_list_t*) * ht->size);
    if (ht->buckets == NULL) {
        fprintf(stderr, "Memory allocation failed during resize\n");
        ht->buckets = old_buckets;
        ht->size = old_size;
        return;
    }
    
    // Inizializza i nuovi buckets
    for (int i = 0; i < ht->size; i++) {
        ht->buckets[i] = create_linked_list();
        if (ht->buckets[i] == NULL) {
            fprintf(stderr, "Failed to create linked list during resize\n");
            // Cleanup e rollback
            for (int j = 0; j < i; j++) {
                free_linked_list(ht->buckets[j]);
            }
            free(ht->buckets);
            ht->buckets = old_buckets;
            ht->size = old_size;
            return;
        }
    }
    
    // Reinserisci tutti gli elementi nella nuova tabella
    for (int i = 0; i < old_size; i++) {
        linked_list_t *list = old_buckets[i];
        if (list->head != NULL) {
            ll_item_t *current = list->head;
            while (current != NULL) {
                ht_item *item = (ht_item *)current->data;
                if (item != NULL) {
                    // Calcola nuovo indice con la nuova dimensione
                    unsigned int index = ht->hash_function(item->key, ht->size);
                    add_to_list(ht->buckets[index], item);
                    ht->count++;
                }
                current = current->next;
            }
        }
    }
    
    // Libera la vecchia struttura
    for (int i = 0; i < old_size; i++) {
        free_linked_list(old_buckets[i]);
    }
    free(old_buckets);
}

void ht_insert(hash_table_t *ht, void *key, void *value) {
    if (ht == NULL || key == NULL) return;
    
    // Controlla se è necessario ridimensionare
    if (ht_load_factor(ht) >= LOAD_FACTOR_THRESHOLD) {
        ht_resize(ht);
    }
    
    unsigned int index = ht->hash_function(key, ht->size);
    linked_list_t *list = ht->buckets[index];
    
    // Controlla se la chiave esiste già
    ll_item_t *current = list->head;
    while (current != NULL) {
        ht_item *item = (ht_item *)current->data;
        if (item != NULL && strcmp((char *)item->key, (char *)key) == 0) {
            // Aggiorna il valore esistente
            item->value = value;
            return;
        }
        current = current->next;
    }
    
    // Crea nuovo item e inseriscilo
    ht_item *new_item = create_ht_item(key, value);
    if (new_item != NULL) {
        add_to_list(list, new_item);
        ht->count++;
    }
}

void *ht_search(hash_table_t *ht, void *key) {
    if (ht == NULL || key == NULL) return NULL;
    
    unsigned int index = ht->hash_function(key, ht->size);
    linked_list_t *list = ht->buckets[index];
    
    ll_item_t *current = list->head;
    while (current != NULL) {
        ht_item *item = (ht_item *)current->data;
        if (item != NULL && strcmp((char *)item->key, (char *)key) == 0) {
            return item->value;
        }
        current = current->next;
    }
    
    return NULL; // Non trovato
}

void ht_delete(hash_table_t *ht, void *key) {
    if (ht == NULL || key == NULL) return;
    
    unsigned int index = ht->hash_function(key, ht->size);
    linked_list_t *list = ht->buckets[index];
    
    ll_item_t *current = list->head;
    while (current != NULL) {
        ht_item *item = (ht_item *)current->data;
        if (item != NULL && strcmp((char *)item->key, (char *)key) == 0) {
            remove_from_list(list, item);
            free_ht_item(item);
            ht->count--;
            return;
        }
        current = current->next;
    }
}

void free_hash_table(hash_table_t *ht) {
    if (ht == NULL) return;
    
    // Libera tutti gli item in tutti i buckets
    for (int i = 0; i < ht->size; i++) {
        if (ht->buckets[i] != NULL) {
            ll_item_t *current = ht->buckets[i]->head;
            while (current != NULL) {
                ht_item *item = (ht_item *)current->data;
                if (item != NULL) {
                    free_ht_item(item);
                }
                current = current->next;
            }
            free_linked_list(ht->buckets[i]);
        }
    }
    
    free(ht->buckets);
    free(ht);
}

int ht_get_count(hash_table_t *ht) {
    return ht ? ht->count : 0;
}

int ht_get_size(hash_table_t *ht) {
    return ht ? ht->size : 0;
}
