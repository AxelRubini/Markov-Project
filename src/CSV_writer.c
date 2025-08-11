#include "../include/CSV_writer.h"

#include "../include/ht_data_manipulation.h"
#include "../include/utils.h"
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

int csv_build_file(const char *filename, hash_table_t *table) {
  if (!filename || !table) {
    fprintf(stderr, "Invalid filename or hash table\n");
    return -1;
  }

  FILE *file = fopen(filename, "w");
  if (!file) {
    fprintf(stderr, "Failed to open file %s for writing\n", filename);
    return -1;
  }
  char *buffer = dmalloc(1024);
  if (!buffer) {
    fprintf(stderr, "Memory allocation failed for buffer\n");
    fclose(file);
    return -1;
  }
  ll_item_t *current = NULL;
  for (int i = 0; i < table->size; ++i) {
    linked_list_t *bucket = table->buckets[i];
    if (bucket == NULL)
      continue;
    current = bucket->head;
    while (current) {
      ht_item *item = (ht_item *)current->data;
      int buffer_size = calculate_buffer_size(item)*2;
      if (buffer_size < 0) {
        fprintf(stderr, "Failed to calculate buffer size for item\n");
        free(buffer);
        fclose(file);
        return -1;
      }
      char *new_buffer = realloc(buffer, buffer_size);
      if (!new_buffer) {
        fprintf(stderr, "Memory allocation failed for buffer\n");
        fclose(file);
        return -1;
      }
      buffer = new_buffer;
      memset(buffer, 0, buffer_size); // Clear the buffer before use
      create_csv_line(item, buffer);
      size_t line_length = strlen(buffer);
      fwrite(buffer, sizeof(char), line_length, file);
      if (ferror(file)) {
        fprintf(stderr, "Error writing to file %s\n", filename);
        free(buffer);
        fclose(file);
        return -1;
      }
      current = current->next;
    }
  }
  free(buffer);
  if (fclose(file) != 0) {
    fprintf(stderr, "Failed to close file %s\n", filename);
    return -1;
  }
  printf("CSV file %s created successfully\n", filename);

  return 0;
}
