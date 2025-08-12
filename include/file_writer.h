#ifndef FILE_WRITER_H
#define FILE_WRITER_H
#include "utf8_tools.h"
#include "utils.h"
#include "next_word_chooser.h"
#include "hash_table.h"


#include <stdio.h>
#include <stdlib.h>



#define GENERATED_TEST_STARTERS {'.','!','?'} 


int write_output_sp(const char *filename, const int num_words,  char *start_sym, hash_table_t *table);
#endif // FILE_WRITER_H





