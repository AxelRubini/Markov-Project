#include "CSV_reader.h"
#include "CSV_writer.h"
#include "both_sp.h"
#include "file_reader.h"
#include "file_writer.h"
#include "modes_utils.h"
#include "utils.h"

#include <stdio.h>

#include <fcntl.h>
#include <unistd.h>

int both_sp(const char *input_txt, const char *output_csv,
	    const char *output_txt, size_t n_words)
{
	int is_stdin = 0;
	const char *validated_input_txt =
		validate_input_file(input_txt, &is_stdin);
	if (validated_input_txt == NULL) {
		fprintf(stderr, "Invalid input file\n");
		return -1;
	}
	int in_fd = open(validated_input_txt, O_RDONLY);
	if (in_fd < 0) {
		perror("Failed to open input file");
		return -1;
	}

	hash_table_t *ht = build_word_hashtable_fd(in_fd);
	if (ht == NULL) {
		fprintf(stderr, "Failed to build hash table from input file\n");
		safe_close(&in_fd);
		return -1;
	}
	safe_close(&in_fd);
	csv_build_file(output_csv, ht);
	free_hash_table(ht);
	if (is_stdin) {
		unlink(TMP_STDIN_FILE); // Remove the temporary file if it was
					// created
	}

	int print_to_stdout = 0;
	if (is_empty(output_txt)) {
		fprintf(stderr,
			"No output file specified, writing to stdout\n");
		print_to_stdout = 1;
		output_txt = TMP_STDOUT_FILE; // Temporary file for stdout
	}
	fprintf(stderr, "Output file: %s\n", output_txt);

	hash_table_t *freq_table = read_csv_file_sp(output_csv);
	if (ht == NULL) {
		fprintf(stderr, "Failed to read CSV file into hash table\n");
		return -1;
	}
	if (n_words == 0) {
		fprintf(stderr, "Number of words to process cannot be zero\n");
		free_hash_table(ht);
		return -1;
	}

	int result = write_output_sp(output_txt, n_words, freq_table);

	if (result < 0) {
		fprintf(stderr, "Failed to send output: %d\n", result);
		free_hash_table(ht);
		if (output_txt != NULL) {
			remove(output_txt); // Remove the output file if it was
					    // created
		}
		free_hash_table(freq_table);
		return result;
	}
	fprintf(stderr, "Output written to %s\n", output_txt);
	free_hash_table(freq_table);

	if (print_to_stdout) {
		// Print the content of the output file to stdout
		if (print_file_content_to_stdout(output_txt) < 0) {
			fprintf(stdout, "Failed to print output to stdout\n");
			unlink(TMP_STDOUT_FILE); // Remove the temporary file

			return -1;
		}
		fprintf(stderr,
			"\nOutput printed to stdout, temporary file will "
			"be removed\n");
		unlink(TMP_STDOUT_FILE); // Remove the temporary file after
					 // printing
	}
	return 0;
}
