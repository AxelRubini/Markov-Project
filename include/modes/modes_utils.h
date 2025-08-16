#ifndef MODES_UTILS_H
#define MODES_UTILS_H

#include <stddef.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdio.h>


const char *validate_input_file(const char *input_txt, int *is_stdin);

/**
* @brief Print file content to stdout.
* @param filename File to read and print.
* @return 0 on success, -1 on error.
*/
int print_file_content_to_stdout(const char *filename);


char *validate_word(const char *word); 


void kill_and_wait(pid_t pid);







#endif // MODES_UTILS_H
