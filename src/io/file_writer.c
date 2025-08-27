#include "file_writer.h"
#include "frame.h"
#include "hash_table.h"
#include "next_word_chooser.h"
#include "utf8_tools.h"
#include "utils.h"

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <fcntl.h>
#include <sys/stat.h>
#include <unistd.h>

/**
 * @brief Check if word is a sentence terminator.
 *
 * @param word Code-point word.
 * @return Non-zero if word is '.', '!' or '?'.
 */
static int is_last_terminator(const int *word)
{
	if (word == NULL) {
		fprintf(stderr, "Word is NULL\n");
		return 0; // Not a terminator
	}
	// Check if the word is one of the valid terminators
	return (word[0] == '.' || word[0] == '!' || word[0] == '?');
}

/**
 * @brief Generate a random symbol from the predefined stat
 *
 * @return Pointer to an array of integers representing the symbol.
 */
static int *generate_random_symbol()
{
	int *symbol = dmalloc(2 * sizeof(int));
	const int charset[] = GENERATED_TEST_STARTERS;
	size_t index = rand() % (ARR_SZ(charset));
	symbol[0] = charset[index];
	symbol[1] = '\0';
	return symbol;
}

/**
 * @brief Provide an interface to choose a word from the hash table
 *
 * @param Hash table to search for the word.
 * @return pointer to an array of codepoints or null.
 */
static int *search_start_word(const hash_table_t *table)
{
	if (table == NULL || table->size == 0) {
		fprintf(stderr, "Hash table is NULL or empty\n");
		return NULL; // No valid start word
	}
	char search_term[MAX_WORD_LENGTH_BYTES] = {0};
	printf("Enter char of the word to search: ");
	fflush(stdout);
	// Read a UTF-8 word from stdin
	while (fgets(search_term, sizeof(search_term), stdin)) {
		if (search_term[0] == '\n') {
			printf("No input found, please enter a word or type "
			       "exit \n");
			continue;
		}
		search_term[strcspn(search_term, "\n")] = '\0';
		if (strcmp(search_term, "exit") == 0) {
			return NULL;
		}
		int *word = utf8_rawbytes_to_cp((const u8 *)search_term,
						strlen(search_term));
		if (word == NULL) {
			fprintf(stderr,
				"Failed to convert input to UTF-8 word\n");
			return NULL; // Error converting input
		}

		printf("Founded Words, just the first 50 matches are shown\n");
		size_t found_count = 0;
		ht_item *matches[50];
		for (size_t i = 0; i < table->size && found_count < 50; i++) {
			linked_list_t *bucket = table->buckets[i];
			if (bucket == NULL) {
				continue;
			}
			ll_item_t *current = bucket->head;
			while (current != NULL && found_count < 50) {
				ht_item *item = (ht_item *)current->data;
				if (utf8_is_sub_string(item->key, word)) {
					matches[found_count++] = item;
					printf("%zu.", found_count);
					fflush(stdout);
					utf8_print_word(item->key,
							STDOUT_FILENO);
					printf("\n");
				}
				current = current->next;
			}
		}
		if (found_count == 0) {
			printf("No match found\n");
		} else {
			printf("\nSelect a word from (1-%zu) or research: ",
			       found_count);
			char choice_str[16];
			if (fgets(choice_str, sizeof(choice_str), stdin)) {
				size_t choiche = strtoul(choice_str, NULL, 10);
				if (choiche > 0 && choiche <= found_count) {
					int *selected_word = utf8_str_deepcopy(
						matches[choiche - 1]->key);
					if (selected_word == NULL) {
						fprintf(stderr,
							"Failed to create "
							"selected word from "
							"input\n");
						return NULL; // Error converting
							     // input
					}
					printf("selected word: ");
					fflush(stdout);
					utf8_print_word(selected_word,
							STDOUT_FILENO);
					printf("\n");
					return selected_word;
				}
			}
		}
		printf("\nSearch again or exit: ");
	}
	return NULL;
}

/**
 * @brief Create a start word from user input or random symbol.
 *
 * @param table Hash table to search for the word.
 * @return Pointer to an array of integers representing the start word or NULL
 * on error.
 */
static int *create_start_word(const hash_table_t *table)
{
	if (table == NULL || table->size == 0) {
		fprintf(stderr, "Hash table is NULL or empty\n");
		return NULL; // No valid start word
	}
	printf("\nChoose a starting word\n");
	printf("Write a substring of a word and too choose one\n");
	printf("Otherwise write \"exit\" to generate_random_start_word\n\n");
	int *word = search_start_word(table);
	if (word == NULL) {
		const size_t max_tries = 50;
		for (size_t tries = 0; tries < max_tries; ++tries) {
			word = generate_random_symbol();
			if (word == NULL) {
				fprintf(stderr, "Failed to create start word "
						"from random symbol\n");
				return NULL; // Error creating start word
			}
			if ((ht_item *)ht_search(table, word) != NULL) {
				utf8_print_word(word, STDOUT_FILENO);
				printf("\n");
				return word;
			}
			free(word);
		}
	} else {
		return word;
	}
	fprintf(stderr, "Failed to create start word from input\n");
	return NULL;
}

int write_output_sp(const char *filename, const uint32_t num_words,
		    hash_table_t *table)
{
	if (filename == NULL || num_words <= 0) {
		fprintf(stderr, "Invalid filename or number of words\n");
		return -1;
	}
	int fd =
		open(filename, O_WRONLY | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR);
	if (fd < 0) {
		perror("write_output_sp: open file");
		return -1;
	}

	next_word_chooser_t *chooser = create_next_word_chooser(table);
	if (chooser == NULL) {
		fprintf(stderr, "Failed to create next word chooser\n");
		safe_close(&fd);
		return -1;
	}

	int *current_word = create_start_word(table);
	if (current_word == NULL) {
		fprintf(stderr, "Failed to create start word l\n");
		next_word_chooser_free(chooser);
		safe_close(&fd);
		return -1;
	}

	uint32_t words_written = 0;
	while (words_written < num_words) {
		int *next_word =
			next_word_chooser_choose(chooser, current_word);
		if (next_word == NULL) {
			fprintf(stderr,
				"No next word found for the current word\n");
			next_word_chooser_free(chooser);
			free(current_word);
			safe_close(&fd);
			return -1;
		}
		if (is_last_terminator(current_word) || words_written == 0) {
			next_word[0] = utf8_char_to_upper(next_word[0]);
		}
		if (!is_last_terminator(next_word) && words_written > 0) {
			write(fd, " ", 1); // Write a space after each word
		}
		utf8_print_word(next_word, fd);
		words_written++;
		next_word[0] = utf8_char_to_lower(
			next_word[0]); // Convert to lowercase
		free((void *)current_word); // Free the previous word
		current_word = next_word; // Update current_word to the new word
	}

	free(current_word);
	next_word_chooser_free(chooser);
	safe_close(&fd);
	return 0; // Success
}

int write_output_mp(int fd, const char *filename)
{
	if (fd < 0 || filename == NULL) {
		fprintf(stderr, "Invalid file descriptor or filename\n");
		return -1;
	}

	int file_fd =
		open(filename, O_WRONLY | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR);
	if (file_fd < 0) {
		perror("write_output_mp: open file");
		return -1;
	}

	char *buff = NULL;
	size_t cap = 0;
	int *last_word = utf8_str_deepcopy(
		(int[]){'.', '\0'}); // Start with period as last word
	if (last_word == NULL) {
		fprintf(stderr, "Failed to create initial last word\n");
		safe_close(&file_fd);
		return -1;
	}

	int words_written = 0;
	while (1) {
		ssize_t frm_st = recv_frame(fd, &buff, &cap);
		if (frm_st < 0) {
			perror("write_output_mp: recv_frame");
			free(buff);
			free(last_word);
			safe_close(&file_fd);
			return -1;
		}
		if (frm_st == 0) {
			break; // End of stream
		}

		// Convert received bytes back to UTF-8 codepoints
		int *current_word =
			utf8_rawbytes_to_cp((const u8 *)buff, frm_st);
		if (current_word == NULL) {
			fprintf(stderr,
				"Failed to convert frame to UTF-8 word\n");
			free(buff);
			free(last_word);
			safe_close(&file_fd);
			return -1;
		}

		// Apply capitalization rules
		if (is_last_terminator(last_word)) {
			current_word[0] = utf8_char_to_upper(current_word[0]);
		}

		// Write space before word (except for first word and after
		// terminators)
		if (words_written > 0 && !is_last_terminator(current_word)) {
			write(file_fd, " ", 1);
		}

		// Write the word to file
		utf8_print_word(current_word, file_fd);
		words_written++;

		// Convert first letter to lowercase for next iteration
		current_word[0] = utf8_char_to_lower(current_word[0]);

		// Update last_word for next iteration
		free(last_word);
		last_word = current_word; // Transfer ownership
	}

	free(buff);
	free(last_word);
	safe_close(&file_fd);
	return 0;
}

int send_output_mp(int fd, hash_table_t *ht, const uint32_t num_words)
{
	if (fd < 0 || num_words <= 0) {
		fprintf(stderr, "Invalid file descriptor, number of words, or "
				"start symbol\n");
		return -1;
	}
	next_word_chooser_t *chooser = create_next_word_chooser(ht);
	if (chooser == NULL) {
		fprintf(stderr, "Failed to create next word chooser\n");
		return -1;
	}

	int *current_word = create_start_word(ht);

	uint32_t words_written = 0;
	char frm_buf[(MAX_WORD_LENGTH - 1) * sizeof(int) + 1] = {
		0}; // Buffer for the frame
	while (words_written < num_words) {
		int *next_word =
			next_word_chooser_choose(chooser, current_word);
		if (next_word == NULL) {
			fprintf(stderr,
				"No next word found for the current word\n");
			next_word_chooser_free(chooser);
			return -1;
		}
		size_t frame_size =
			utf8_cp_to_rawbytes(next_word, (u8 *)frm_buf);
		if (frame_size == 0) {
			fprintf(stderr,
				"Failed to convert next word to raw bytes\n");
			next_word_chooser_free(chooser);
			return -1;
		}
		if (send_frame(fd, frm_buf, frame_size) < 0) {
			perror("send_output_mp: send_frame");
			next_word_chooser_free(chooser);
			return -1;
		}
		free((void *)current_word); // Free the previous word
		current_word = next_word; // Update current_word to the new word
		words_written++;
	}
	free((void *)current_word); // Free the last word
	next_word_chooser_free(chooser); // Free the chooser
	return 0; // Success
}
