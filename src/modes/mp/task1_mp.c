#include "task1_mp.h"
#include "CSV_writer.h"
#include "file_reader_mp.h"
#include "hash_table.h"
#include "modes_utils.h"
#include "utils.h"

#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
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
  if (publish_word_on_pipe(fd, pipe_to_write) < 0) {
    fprintf(stderr, "Error reading input file\n");
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

static int w_p2(int pipe_to_read, int pipe_to_write) {
  if (pipe_to_read < 0 || pipe_to_write < 0) {
    fprintf(stderr, "Invalid pipe descriptors for reading or writing\n");
    return -1;
  }
  hash_table_t *ht =
      create_hash_table(HASH_TABLE_SIZE, word_hash, word_hashtable_keycmp);
  if (!ht) {
    fprintf(stderr, "Failed to create hash table\n");
    safe_close(&pipe_to_read);
    safe_close(&pipe_to_write);
    return -1;
  }
  int rw_res = read_words_from_pipe(pipe_to_read, ht);
  if (rw_res < 0) {
    fprintf(stderr, "Error reading words from pipe\n");
    free_hash_table(ht);
    safe_close(&pipe_to_read);
    safe_close(&pipe_to_write);
    return -1;
  }
  safe_close(&pipe_to_read);
  if (csv_line_send(pipe_to_write, ht) < 0) {
    fprintf(stderr, "Error writing CSV lines to pipe\n");
    free_hash_table(ht);
    safe_close(&pipe_to_write);
    return -1;
  }
  free_hash_table(ht);
  if (safe_close(&pipe_to_write) < 0)
    return -1;
  return 0;
}

static int w_p3(int pipe_to_read, const char *output_csv) {
  if (pipe_to_read < 0 || !output_csv) {
    fprintf(stderr, "Invalid arguments to p3\n");
    return -1;
  }
  if (csv_line_recv(pipe_to_read, output_csv) < 0) {
    fprintf(stderr, "Error receiving CSV lines\n");
    safe_close(&pipe_to_read);
    return -1;
  }
  if (safe_close(&pipe_to_read) < 0)
    return -1;
  return 0;
}

int task1_mp(const char *input_txt, const char *output_csv) {
  int status = -1;
  int is_stdin = 0;
  const char *validated_input_txt = validate_input_file(input_txt, &is_stdin);
  if (!validated_input_txt) {
    fprintf(stderr, "Invalid input file\n");
    goto done;
  }

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
    if (w_p1(pipe12[1], validated_input_txt) < 0)
      _exit(1);
    _exit(0);
  }

  p2 = fork();
  if (p2 < 0)
    goto fail_kill_p1;
  if (p2 == 0) {
    int not_p2[] = {pipe12[1], pipe23[0]};
    safe_close_pipes(not_p2, ARR_SZ(not_p2));
    if (w_p2(pipe12[0], pipe23[1]) < 0)
      _exit(1);
    _exit(0);
  }

  p3 = fork();
  if (p3 < 0)
    goto fail_kill_p1p2;
  if (p3 == 0) {
    int not_p3[] = {pipe12[0], pipe12[1], pipe23[1]};
    safe_close_pipes(not_p3, ARR_SZ(not_p3));
    if (w_p3(pipe23[0], output_csv) < 0)
      _exit(1);
    _exit(0);
  }

  safe_close_pipes(all_pipes, ARR_SZ(all_pipes));
  int st;
  while (waitpid(-1, &st, 0) > 0)
    ;
  status = 0;
  goto done;

fail_kill_p1p2:
  kill_and_wait(p2);
fail_kill_p1:
  kill_and_wait(p1);
fail_pipes:
  safe_close_pipes(all_pipes, ARR_SZ(all_pipes));
done:
  if (is_stdin)
    unlink(validated_input_txt);
  return status;
}

