#include "task2_mp.h"
#include "CSV_reader.h"
#include "file_writer.h"
#include "hash_table.h"
#include "modes_utils.h"
#include "utils.h"

#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

static int w_p1(int pipe_to_write, const char *input_csv) {
  if (pipe_to_write < 0 || !input_csv) {
    fprintf(stderr, "Invalid arguments to p1\n");
    return -1;
  }
  int fd = open(input_csv, O_RDONLY);
  if (fd < 0) {
    perror("Failed to open input CSV file");
    return -1;
  }
  int result = publish_line_on_pipe(fd, pipe_to_write);
  if (result < 0) {
    fprintf(stderr, "Error reading CSV file\n");
    safe_close(&fd);
    safe_close(&pipe_to_write);
    return -1;
  }
  if (safe_close(&fd) < 0)
    return -1;
  if (safe_close(&pipe_to_write) < 0)
    return -1;
  return 0;
}

static int w_p2(int pipe_to_read, int pipe_to_write, size_t n_words,
                const char *word) {
  if (pipe_to_read < 0 || pipe_to_write < 0) {
    fprintf(stderr, "Invalid pipe descriptors for reading or writing\n");
    return -1;
  }
  hash_table_t *ht = read_lines_from_pipe(pipe_to_read);
  if (!ht) {
    fprintf(stderr, "Error reading lines from pipe\n");
    safe_close(&pipe_to_read);
    safe_close(&pipe_to_write);
    return -1;
  }
  if (safe_close(&pipe_to_read) < 0) {
    free_hash_table(ht);
    return -1;
  }
  int result = send_output_mp(pipe_to_write, ht, n_words, word);
  if (result < 0) {
    fprintf(stderr, "Error writing lines to pipe\n");
    free_hash_table(ht);
    safe_close(&pipe_to_write);
    return -1;
  }
  free_hash_table(ht);
  if (safe_close(&pipe_to_write) < 0)
    return -1;
  return 0;
}

static int w_p3(int pipe_to_read, const char *output_txt) {
  if (pipe_to_read < 0) {
    fprintf(stderr, "Invalid pipe descriptor for reading\n");
    return -1;
  }
  int fd =
      open(output_txt, O_WRONLY | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR);
  if (fd < 0) {
    perror("Failed to open output file for writing");
    return -1;
  }
  safe_close(&fd);
  if (write_output_mp(pipe_to_read, output_txt) < 0) {
    fprintf(stderr, "Error reading from pipe and writing to file\n");
    return -1;
  }
  return 0;
}

int task2_mp(const char *input_csv, const char *output_txt, size_t n_words,
             const char *word) {
  int status = -1;
  int print_to_stdout = 0;
  if (is_empty(output_txt)) {
    output_txt = "tmp_output.txt";
    print_to_stdout = 1;
  }

  char *validated_word = validate_word(word);
  if (!validated_word)
    goto done;

  int pipe12[2], pipe23[2];
  if (pipe(pipe12) < 0 || pipe(pipe23) < 0) {
    perror("pipe");
    goto done;
  }

  int all_pipes[] = {pipe12[0], pipe12[1], pipe23[0], pipe23[1]};
  pid_t p1 = -1, p2 = -1, p3 = -1;

  p1 = fork();
  if (p1 < 0)
    goto fail_pipes;
  if (p1 == 0) {
    int not_p1[] = {pipe12[0], pipe23[0], pipe23[1]};
    safe_close_pipes(not_p1, ARR_SZ(not_p1));
    if (w_p1(pipe12[1], input_csv) < 0)
      _exit(1);
    _exit(0);
  }

  p2 = fork();
  if (p2 < 0)
    goto fail_kill_p1;
  if (p2 == 0) {
    int not_p2[] = {pipe12[1], pipe23[0]};
    safe_close_pipes(not_p2, ARR_SZ(not_p2));
    if (w_p2(pipe12[0], pipe23[1], n_words, validated_word) < 0)
      _exit(1);
    _exit(0);
  }

  p3 = fork();
  if (p3 < 0)
    goto fail_kill_p1p2;
  if (p3 == 0) {
    int not_p3[] = {pipe12[0], pipe12[1], pipe23[1]};
    safe_close_pipes(not_p3, ARR_SZ(not_p3));
    if (w_p3(pipe23[0], output_txt) < 0)
      _exit(1);
    _exit(0);
  }

  safe_close_pipes(all_pipes, ARR_SZ(all_pipes));
  int st;
  while (waitpid(-1, &st, 0) > 0)
    ;
  if (print_to_stdout) {
    print_file_content_to_stdout(output_txt);
    unlink(output_txt);
  }
  status = 0;
  goto done;

fail_kill_p1p2:
  kill_and_wait(p2);
fail_kill_p1:
  kill_and_wait(p1);
fail_pipes:
  safe_close_pipes(all_pipes, ARR_SZ(all_pipes));
done:
  if (validated_word && validated_word != word)
    free(validated_word);
  return status;
}

