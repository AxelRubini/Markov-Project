#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <unistd.h>

// --------- Replace with your real functions (fd-based) ----------
static int stage1_reader_task1(int input_fd, int out_fd) {
  // TODO: read UTF-8 text from input_fd, write raw/normalized stream to out_fd
  // Example passthrough (remove):
  char buf[4096];
  ssize_t n;
  while ((n = read(input_fd, buf, sizeof(buf))) > 0) {
    if (write(out_fd, buf, (size_t)n) != n)
      return 1;
  }
  return (n < 0) ? 1 : 0;
}

static int stage2_builder_task1(int in_fd, int out_fd) {
  (void)in_fd;
  (void)out_fd;
  // TODO: parse tokens from in_fd, build hash table, write CSV to out_fd
  return 0;
}

static int stage1_reader_task2(int input_csv_fd, int out_fd) {
  // TODO: read CSV from input_csv_fd, forward or pre-validate to out_fd
  char buf[4096];
  ssize_t n;
  while ((n = read(input_csv_fd, buf, sizeof(buf))) > 0) {
    if (write(out_fd, buf, (size_t)n) != n)
      return 1;
  }
  return (n < 0) ? 1 : 0;
}

static int stage2_generator_task2(int in_fd, int out_fd, int words_to_gen,
                                  const char *prev_word) {
  // TODO: rebuild distribution from CSV read on in_fd, generate to out_fd
  (void)in_fd;
  (void)out_fd;
  (void)words_to_gen;
  (void)prev_word;
  return 0;
}

static int stage3_writer(int in_fd, int output_fd) {
  // TODO: read from in_fd, write to output_fd
  char buf[4096];
  ssize_t n;
  while ((n = read(in_fd, buf, sizeof(buf))) > 0) {
    if (write(output_fd, buf, (size_t)n) != n)
      return 1;
  }
  return (n < 0) ? 1 : 0;
}
// ---------------------------------------------------------------

static void xclose(int fd) {
  if (close(fd) == -1) {
    perror("close");
    _exit(1);
  }
}

int run_mp_fd_mode(int task,             // 1 or 2
                   const char *in_path,  // input path (text or CSV)
                   const char *out_path, // output path (CSV or text)
                   int words_to_gen,     // only for task==2
                   const char *prev_word // may be NULL for task==2
) {
  int pipeA[2], pipeB[2];
  if (pipe(pipeA) == -1) {
    perror("pipeA");
    return 1;
  }
  if (pipe(pipeB) == -1) {
    perror("pipeB");
    xclose(pipeA[0]);
    xclose(pipeA[1]);
    return 1;
  }

  // Open files (only in parent; pass fds to children)
  int in_fd = open(in_path, O_RDONLY);
  if (in_fd < 0) {
    perror("open input");
    xclose(pipeA[0]);
    xclose(pipeA[1]);
    xclose(pipeB[0]);
    xclose(pipeB[1]);
    return 1;
  }
  int out_fd = open(out_path, O_CREAT | O_TRUNC | O_WRONLY, 0644);
  if (out_fd < 0) {
    perror("open output");
    xclose(in_fd);
    xclose(pipeA[0]);
    xclose(pipeA[1]);
    xclose(pipeB[0]);
    xclose(pipeB[1]);
    return 1;
  }

  pid_t r = fork();
  if (r == -1) {
    perror("fork reader");
    return 1;
  }
  if (r == 0) {
    // ---- Reader child ----
    // Writes to pipeA[1]
    xclose(pipeA[0]); // not used
    xclose(pipeB[0]);
    xclose(pipeB[1]); // not used
    // in_fd already opened; call your reader with (in_fd -> pipeA[1])
    int rc = (task == 1) ? stage1_reader_task1(in_fd, pipeA[1])
                         : stage1_reader_task2(in_fd, pipeA[1]);
    xclose(in_fd);
    xclose(pipeA[1]); // signal EOF downstream
    _exit(rc);
  }

  pid_t b = fork();
  if (b == -1) {
    perror("fork builder");
    return 1;
  }
  if (b == 0) {
    // ---- Builder/Generator child ----
    xclose(pipeA[1]); // read end only
    xclose(pipeB[0]); // write end only
    xclose(in_fd);    // not used here
    xclose(out_fd);   // not used here
    int rc = (task == 1) ? stage2_builder_task1(pipeA[0], pipeB[1])
                         : stage2_generator_task2(pipeA[0], pipeB[1],
                                                  words_to_gen, prev_word);
    xclose(pipeA[0]);
    xclose(pipeB[1]); // signal EOF to writer
    _exit(rc);
  }

  pid_t w = fork();
  if (w == -1) {
    perror("fork writer");
    return 1;
  }
  if (w == 0) {
    // ---- Writer child ----
    xclose(pipeA[0]);
    xclose(pipeA[1]); // not used
    xclose(pipeB[1]); // read end only
    xclose(in_fd);    // not used
    int rc = stage3_writer(pipeB[0], out_fd);
    xclose(pipeB[0]);
    xclose(out_fd);
    _exit(rc);
  }

  // ---- Parent ----
  xclose(in_fd);
  xclose(out_fd);
  xclose(pipeA[0]);
  xclose(pipeA[1]);
  xclose(pipeB[0]);
  xclose(pipeB[1]);

  int sr = 0, sb = 0, sw = 0;
  waitpid(r, &sr, 0);
  waitpid(b, &sb, 0);
  waitpid(w, &sw, 0);

  int rc = 0;
  if (WIFEXITED(sr) && WEXITSTATUS(sr) != 0)
    rc = 1;
  if (WIFEXITED(sb) && WEXITSTATUS(sb) != 0)
    rc = 1;
  if (WIFEXITED(sw) && WEXITSTATUS(sw) != 0)
    rc = 1;
  return rc;
}

// Example CLI glue (merge into your main):
int main(int argc, char **argv) {
  if (argc < 5) {
      fprintf(stderr,
              "Usage:\n  %s mp 1 <input_txt> <output_csv>\n  %s mp 2 <input_csv> "
              "<output_txt> <nWords> [prevWord]\n",
              argv[0], argv[0]);
    return 1;
  }
  if (strcmp(argv[1], "mp") != 0) {
      fprintf(stderr, "Use 'mp' for multiprocess mode\n");
    return 1;
  }
  int task = atoi(argv[2]);
  if (task == 1) {
    return run_mp_fd_mode(1, argv[3], argv[4], 0, NULL);
  } else if (task == 2) {
    if (argc < 6) {
        fprintf(stderr, "Missing nWords for task 2\n");
      return 1;
    }
    int n = atoi(argv[5]);
    const char *prev = (argc >= 7) ? argv[6] : NULL;
    return run_mp_fd_mode(2, argv[3], argv[4], n, prev);
  }
    fprintf(stderr, "Task must be 1 or 2\n");
  return 1;
}
