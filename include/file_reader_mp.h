#include "../include/hash_table.h"
#include "../include/utf8_tools.h"
#include "../include/utils.h"
#include "../include/word.h"

#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>


int publish_word_on_pipe(int fd, int wfd);
int read_words_from_pipe(int rfd, hash_table_t *ht);
