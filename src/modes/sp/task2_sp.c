#include "CSV_reader.h"
#include "file_writer.h"
#include "modes_utils.h"
#include "task2_sp.h"

#include <stdio.h>

#include <unistd.h>

int task2_sp(const char *input_csv, const char *output_txt, size_t n_words)
{
	int print_to_stdout = 0;
	if (is_empty(output_txt)) {
		fprintf(stderr,
			"No output file specified, writing to stdout\n");
		print_to_stdout = 1;
		output_txt = TMP_STDOUT_FILE; // Temporary file for stdout
	}

	hash_table_t *ht = read_csv_file_sp(input_csv);
	if (ht == NULL) {
		fprintf(stderr, "Failed to read CSV file into hash table\n");
		return -1;
	}
	if (n_words == 0) {
		fprintf(stderr, "Number of words to process cannot be zero\n");
		free_hash_table(ht);
		return -1;
	}

	int result = write_output_sp(output_txt, n_words, ht);
	if (result < 0) {
		fprintf(stderr, "Failed to send output: %d\n", result);
		free_hash_table(ht);
		if (output_txt != NULL) {
			remove(output_txt); // Remove the output file if it was
					    // created
		}
		free_hash_table(ht);
		return result;
	}
	fprintf(stderr, "Output written to %s\n", output_txt);
	free_hash_table(ht);

	if (print_to_stdout) {
		// Print the content of the output file to stdout
		if (print_file_content_to_stdout(output_txt) < 0) {
			fprintf(stderr, "Failed to print output to stdout\n");
			unlink(output_txt); // Remove the temporary file
			return -1;
		}
		fprintf(stdout,
			"\nOutput printed to stdout, temporary file will "
			"be removed\n");
		unlink(output_txt); // Remove the temporary file after printing
	}
	return 0;
}
