#include "both_mp.h"
#include "CSV_reader.h"
#include "CSV_writer.h"
#include "file_reader_mp.h"
#include "file_writer.h"
#include "hash_table.h"
#include "modes_utils.h"
#include "utils.h"
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

static int w_p1(int pipe_to_write, const char *input_txt) {
  if (pipe_to_write < 0) {
    fprintf(stderr, "Invalid pipe descriptor for writing\n");
    return -1;
  }
  int fd = open(input_txt, O_RDONLY);
  if (fd < 0) {
    perror("Failed to open input file");
    return -1;
  }
  int pb_w_res = publish_word_on_pipe(fd, pipe_to_write);
  if (pb_w_res < 0) {
    fprintf(stderr, "Error reading input file\n");
    close(fd);
    close(pipe_to_write);
    return -1;
  }
  if (close(fd) < 0) {
    perror("Failed to close input file");
    close(pipe_to_write);
    return -1;
  }
  if (close(pipe_to_write) < 0) {
    perror("Failed to close pipe after writing");
    return -1;
  }
  return 0;
}

// Process 2_1 sends csv lines to P3
static int w_p2_1(int pipe_to_write_csv, hash_table_t *ht) {
  if (ht == NULL) {
    fprintf(stderr, "Hash table is NULL\n");
    return -1;
  }
  if (pipe_to_write_csv < 0) {
    fprintf(stderr, "Invalid pipe descriptor for writing CSV lines\n");
    return -1;
  }
  if (ht->size == 0) {
    fprintf(stderr, "Hash table is empty, nothing to write\n");
    return 0;
  }
  int result = csv_line_send(pipe_to_write_csv, ht);
  if (result < 0) {
    fprintf(stderr, "Error writing CSV lines to pipe\n");
    return -1;
  }
  if (close(pipe_to_write_csv) < 0) {
    perror("Failed to close pipe after writing CSV lines");
    return -1;
  }
  safe_close(&pipe_to_write_csv);
  return 0;
}

// process p2_2 sends csv lines to p4
static int w_p2_2(int pipe_to_write_ts2, hash_table_t *ht) {
  if (ht == NULL) {
    fprintf(stderr, "Hash table is NULL\n");
    return -1;
  }
  if (pipe_to_write_ts2 < 0) {
    fprintf(stderr, "Invalid pipe descriptor for writing to task 2_2\n");
    return -1;
  }
  if (ht->size == 0) {
    fprintf(stderr, "Hash table is empty, nothing to write\n");
    return 0;
  }
  int result = csv_line_send(pipe_to_write_ts2, ht);
  if (result < 0) {
    fprintf(stderr, "Error writing CSV lines to pipe for task 2_2\n");
    return -1;
  }
  if (close(pipe_to_write_ts2) < 0) {
    perror("Failed to close pipe after writing task 2_2");
    return -1;
  }
  safe_close(&pipe_to_write_ts2);
  return 0;
}

// Process 2 reads word from pipe and creates a hash table with word frequencies
static int w_p2(int pipe_to_read, int pipe_to_write_csv,
                int pipe_to_write_ts2) {
  if (pipe_to_read < 0 || pipe_to_write_csv < 0 || pipe_to_write_ts2 < 0) {
    fprintf(stderr, "Invalid pipe descriptors for reading or writing\n");
    return -1;
  }
  hash_table_t *ht =
      create_hash_table(HASH_TABLE_SIZE, word_hash, word_hashtable_keycmp);
  if (ht == NULL) {
    fprintf(stderr, "Failed to create hash table\n");
    safe_close(&pipe_to_write_csv);
    safe_close(&pipe_to_write_ts2);
    return -1;
  }
  int rw_res = read_words_from_pipe(pipe_to_read, ht);
  if (rw_res < 0) {
    fprintf(stderr, "Error reading words from pipe\n");
    free_hash_table(ht);
    safe_close(&pipe_to_write_csv);
    safe_close(&pipe_to_write_ts2);
    return -1;
  }
  safe_close(&pipe_to_read);
  if (ht->size == 0) {
    fprintf(stderr, "Hash table is empty, nothing to write\n");
    free_hash_table(ht);
    close(pipe_to_write_csv);
    close(pipe_to_write_ts2);
    return 0;
  }

  int p_2_1 = fork();
  if (p_2_1 < 0) {
    perror("Failed to fork process for writing CSV lines");
    free_hash_table(ht);
    safe_close(&pipe_to_write_csv);
    safe_close(&pipe_to_write_ts2);
    return -1;
  }
  if (p_2_1 == 0) {
    // Child process for writing CSV lines
    safe_close(&pipe_to_write_ts2); // Child 2_1 doesn't need pipe to task 2_2
    if (w_p2_1(pipe_to_write_csv, ht) < 0) {
      fprintf(stderr, "Child process w_p2_1 failed\n");
      free_hash_table(ht);
      _exit(-1);
    }
    free_hash_table(ht);
    _exit(0);
  }

  int p_2_2 = fork();
  if (p_2_2 < 0) {
    perror("Failed to fork process for writing task 2_2");
    free_hash_table(ht);
    kill(p_2_1, SIGTERM); // Terminate first child
    safe_close(&pipe_to_write_csv);
    safe_close(&pipe_to_write_ts2);
    return -1;
  }
  if (p_2_2 == 0) {
    // Child process for writing task 2_2
    safe_close(&pipe_to_write_csv); // Child 2_2 doesn't need pipe to CSV
    if (w_p2_2(pipe_to_write_ts2, ht) < 0) {
      fprintf(stderr, "Child process w_p2_2 failed\n");
      free_hash_table(ht);
      _exit(-1);
    }
    free_hash_table(ht);
    _exit(0);
  }

  // Parent closes pipes after forking
  safe_close(&pipe_to_write_csv);
  safe_close(&pipe_to_write_ts2);

  // Parent process waits for child processes to finish
  int st;
  while (waitpid(-1, &st, 0) > 0);
  free_hash_table(ht);
  return 0;
}

// process p3 reads from pipe csv lines and writes to csv file
static int w_p3(int pipe_to_read, const char *output_csv) {
  if (pipe_to_read < 0 || !output_csv) {
    fprintf(stderr, "Invalid arguments to p3\n");
    return 1;
  }

  if (csv_line_recv(pipe_to_read, output_csv) < 0) {
    fprintf(stderr, "p3: csv_line_recv failed\n");
    close(pipe_to_read);
    return 2;
  }

  close(pipe_to_read);
  return 0;
}

// Process p4 build a hash table key-nxt_word prob and choose the word
static int w_p4(int pipe_to_read, int pipe_to_write, size_t n_words,
                const char *word) {
  if (pipe_to_read < 0 || pipe_to_write < 0) {
    fprintf(stderr, "Invalid pipe descriptors for reading or writing\n");
    return -1;
  }
  hash_table_t *ht = read_lines_from_pipe(pipe_to_read);
  if (ht == NULL) {
    fprintf(stderr, "Error reading lines from pipe\n");
    return -1;
  }
  if (close(pipe_to_read) < 0) {
    perror("Failed to close pipe after reading");
    free_hash_table(ht);
    return -1;
  }
  int result = send_output_mp(pipe_to_write, ht, n_words, word);
  if (result < 0) {
    fprintf(stderr, "Error writing lines to pipe\n");
    free_hash_table(ht);
    return -1;
  }
  if (close(pipe_to_write) < 0) {
    perror("Failed to close pipe after writing");
    free_hash_table(ht);
    return -1;
  }
  free_hash_table(ht);
  return 0;
}

// Process 5 reads word from pipe and writes to output file
static int w_p5(int pipe_to_read, const char *output_txt) {
  if (pipe_to_read < 0) {
    fprintf(stderr, "Invalid pipe descriptor for reading\n");
    return -1;
  }
  int fd = open(output_txt, O_WRONLY | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR);
  if (fd < 0) {
    perror("Failed to open output file for writing");
    return -1;
  }
  safe_close(&fd);
  int result = write_output_mp(pipe_to_read, output_txt);
  if (result < 0) {
    fprintf(stderr, "Error reading from pipe and writing to file\n");
    close(fd);
    return -1;
  }
  return 0;
}

int both_mp(const char *input_txt, const char *output_csv,
            const char *output_txt, size_t n_words, const char *word) {
  int status = -1; // valore di ritorno in caso di errore
  int print_to_stdout = 0;

  // --- Validazioni input/word
  int is_stdin = 0;
  char *validated_input_txt = validate_input_file(input_txt, &is_stdin);
  char *validated_word = validate_word(word);
  if (!validated_input_txt || !validated_word) {
    fprintf(stderr, "Invalid input/word\n");
    goto fail_input_word;
  }

  if (!output_txt) {
    output_txt = "tmp_output.txt";
    print_to_stdout = 1;
  }

  // --- Pipe
  int pipe_read[2], pipe_csv[2], pipe_freqtab[2], pipe_out[2];
  if (pipe(pipe_read) < 0 || pipe(pipe_csv) < 0 || pipe(pipe_freqtab) < 0 ||
      pipe(pipe_out) < 0) {
    perror("pipe");
    goto fail_input_word;
  }

  int all_pipes[] = {pipe_read[0], pipe_read[1], pipe_csv[0], pipe_csv[1],
                     pipe_freqtab[0], pipe_freqtab[1], pipe_out[0], pipe_out[1]};

  // --- Forks
  pid_t p1 = -1, p2 = -1, p3 = -1, p4 = -1, p5 = -1;

  // P1
  p1 = fork();
  if (p1 < 0) {
    perror("fork p1");
    goto fail_pipes;
  }
  if (p1 == 0) {
    // Child P1: read input -> pipe12[1]
    close(pipe_read[0]);
    int not_p1[] = {pipe_csv[0], pipe_csv[1], pipe_freqtab[0],
                    pipe_freqtab[1], pipe_out[0], pipe_out[1]};
    safe_close_pipes(not_p1, ARR_SZ(not_p1));
    if (w_p1(pipe_read[1], validated_input_txt) < 0)
      _exit(1);
    _exit(0);
  }

  // P2
  p2 = fork();
  if (p2 < 0) {
    perror("fork p2");
    goto fail_kill_p1;
  }
  if (p2 == 0) {
    int not_p2[] = {pipe_csv[0], pipe_freqtab[0],pipe_out[0], pipe_out[1],pipe_read[1]};
    safe_close_pipes(not_p2, ARR_SZ(not_p2));
    if (w_p2(pipe_read[0], pipe_csv[1], pipe_freqtab[1]) < 0)
      _exit(1);
    _exit(0);
  }

  // P3
  p3 = fork();
  if (p3 < 0) {
    perror("fork p3");
    goto fail_kill_p1p2;
  }
  if (p3 == 0) {
    int not_p3[] = {pipe_read[0], pipe_read[1], pipe_freqtab[0],
                    pipe_freqtab[1], pipe_out[0], pipe_out[1],pipe_csv[1]};
    safe_close_pipes(not_p3, ARR_SZ(not_p3));
    if (w_p3(pipe_csv[0], output_csv) < 0)
      _exit(1);
    _exit(0);
  }

  // P4
  p4 = fork();
  if (p4 < 0) {
    perror("fork p4");
    goto fail_kill_p1p2p3;
  }
  if (p4 == 0) {
    int not_p4[] = {pipe_read[0], pipe_read[1], pipe_csv[0], pipe_csv[1], pipe_out[0],
     pipe_freqtab[1]};
    safe_close_pipes(not_p4, ARR_SZ(not_p4));
    if (w_p4(pipe_freqtab[0], pipe_out[1], n_words, validated_word) < 0)
      _exit(1);
    _exit(0);
  }

  // P5
  p5 = fork();
  if (p5 < 0) {
    perror("fork p5");
    goto fail_kill_p1p2p3p4;
  }
  if (p5 == 0) {
    // Child P5: read da pipe45[0] -> output_txt
    int not_p5[] = {pipe_read[0], pipe_read[1], pipe_csv[0],
                    pipe_csv[1], pipe_freqtab[0], pipe_freqtab[1],pipe_out[1]};
    safe_close_pipes(not_p5, ARR_SZ(not_p5));
    if (w_p5(pipe_out[0], output_txt) < 0)
      _exit(1);
    _exit(0);
  }

  // --- Padre: chiude tutto e attende
  safe_close_pipes(all_pipes, ARR_SZ(all_pipes));

  // Wait children
  int st;
  while (waitpid(-1,&st,0) > 0);


  // Stampa su stdout se richiesto
  if (print_to_stdout) {
    FILE *f = fopen(output_txt, "r");
    if (f) {
      char buf[1024];
      while (fgets(buf, sizeof buf, f))
        fputs(buf, stdout);
      fclose(f);
      unlink(output_txt);
    }
  }

  status = 0; // success
  goto done;

fail_kill_p1p2p3p4:
  kill_and_wait(p4);
fail_kill_p1p2p3:
  kill_and_wait(p3);
fail_kill_p1p2:
  kill_and_wait(p2);
fail_kill_p1:
  kill_and_wait(p1);
fail_pipes:
  safe_close_pipes(all_pipes, ARR_SZ(all_pipes));
fail_input_word:; // no-op

done:
  free(validated_input_txt);
  free(validated_word);
  return status;
}
