#include "task1_mp.h"
#include "CSV_writer.h"
#include "file_reader_mp.h"
#include "hash_table.h"
#include "word.h"

#include <fcntl.h>
#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/wait.h>
#include <signal.h>
#include <stdlib.h>


static void die(const char *msg) {
  perror(msg);
  _exit(1);
}

static int w_p1(int pipe_to_write, const char *input_txt) {
  if (pipe_to_write < 0 || !input_txt) {
    fprintf(stderr, "Invalid arguments to p1\n");
    return 1;
  }

  int fd = open(input_txt, O_RDONLY);
  if (fd < 0) {
    perror("open input file");
    return 2;
  }

  if (publish_word_on_pipe(fd, pipe_to_write) < 0) {
    fprintf(stderr, "p1: publish_word_on_pipe failed\n");
    close(fd);
    close(pipe_to_write);
    return 3;
  }

  close(fd);
  close(pipe_to_write); // Signal EOF to p2
  return 0;
}

static int w_p2(int pipe_to_read, int pipe_to_write) {
  if (pipe_to_read < 0 || pipe_to_write < 0) {
    fprintf(stderr, "Invalid arguments to p2\n");
    return 1;
  }

  hash_table_t *ht = create_hash_table(HASH_TABLE_SIZE, word_hash, word_hashtable_keycmp);
  if (!ht) {
    fprintf(stderr, "p2: create_hash_table failed\n");
    close(pipe_to_read);
    close(pipe_to_write);
    return 2;
  }

  int rw_st = read_words_from_pipe(pipe_to_read, ht);
  if (rw_st < 0) {
    fprintf(stderr, "p2: read_words_from_pipe failed\n");
    close(pipe_to_read);
    close(pipe_to_write);
    return 1;
  }
  close(pipe_to_read); // Close immediately after reading

  if (!ht) {
    fprintf(stderr, "p2: read_words_from_pipe failed\n");
    close(pipe_to_write);
    return 2;
  }

  if (csv_line_send(pipe_to_write, ht) < 0) {
    fprintf(stderr, "p2: csv_line_send failed\n");
    close(pipe_to_write);
    free_hash_table(ht);
    return 3;
  }

  free_hash_table(ht);
  close(pipe_to_write);
  return 0;
}

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


int task1_mp(const char *input_txt, const char *output_csv) {

  if (input_txt == NULL) {

    int fd = open("tmp.txt", O_WRONLY | O_CREAT | O_TRUNC, 0644);

    if (fd < 0) {
      perror("Failed to open temporary file");
      return -1;
    }

    // Scan standard input and write to the temporary file
    char buffer[1024];
    ssize_t bytes_read;
    while ((bytes_read = read(STDIN_FILENO, buffer, sizeof(buffer))) > 0) {
      if (write(fd, buffer, bytes_read) < 0) {
        perror("Failed to write to temporary file");
        close(fd);
        return -1;
      }
    }
    close(fd);
  }
  int pipe12[2], pipe23[2];
  if (pipe(pipe12) != 0) {
    perror("pipe12");
    return 1;
  }
  if (pipe(pipe23) != 0) {
    perror("pipe23");
    close(pipe12[0]); // Clean up pipe12 in case of error
    close(pipe12[1]);
    return 1;
  }
  pid_t p1 = fork();
  if (p1 < 0) {
    perror("fork p1");
    // Close all pipes on error
    close(pipe12[0]);
    close(pipe12[1]);
    close(pipe23[0]);
    close(pipe23[1]);
    return 1;
  }
  if (p1 == 0) {
    // Child p1: writer -> pipe12
    close(pipe12[0]); // read end unused
    close(pipe23[0]); // these pipes unused
    close(pipe23[1]);
    int rc = w_p1(pipe12[1], input_txt);
    if (rc != 0) {
      fprintf(stderr, "p1 failed with error code %d\n", rc);
      close(pipe12[1]);
      _exit(rc);
    }
    _exit(0); // child p1 already closes pipe12[1]
  }
  pid_t p2 = fork();
  if (p2 < 0) {
    perror("fork p2");
    // Terminate P1 and close pipes
    kill(p1, SIGTERM);
    close(pipe12[0]);
    close(pipe12[1]);
    close(pipe23[0]);
    close(pipe23[1]);
    return 1;
  }
  if (p2 == 0) {
    // Child p2: read word pairs from pipe12[0], insert into HT,
    // then send CSV lines on pipe23[1]
    close(pipe12[1]); // write end unused
    close(pipe23[0]); // read end unused
    int rc = w_p2(pipe12[0], pipe23[1]);
    if (rc != 0) {
      fprintf(stderr, "p2 failed with error code %d\n", rc);
      close(pipe12[0]);
      close(pipe23[1]);
      _exit(rc);
    }
    _exit(0); // child p2 already closes its fds
  }
  pid_t p3 = fork();
  if (p3 < 0) {
    perror("fork p3");
    // Terminate P1 and P2, close pipes
    kill(p1, SIGTERM);
    kill(p2, SIGTERM);
    close(pipe12[0]);
    close(pipe12[1]);
    close(pipe23[0]);
    close(pipe23[1]);
    return 1;
  }
  if (p3 == 0) {
    // Child p3: read CSV line from pipe23[0] and write to output_csv
    close(pipe12[0]); // pipe12 unused
    close(pipe12[1]);
    close(pipe23[1]); // write end unused
    int rc = w_p3(pipe23[0], output_csv);
    if (rc != 0) {
      fprintf(stderr, "p3 failed with error code %d\n", rc);
      close(pipe23[0]);
      _exit(rc);
    }
    _exit(0); // child p3 already closes pipe23[0]
  }
  // Parent process: close all unused ends and wait
  close(pipe12[0]);
  close(pipe12[1]);
  close(pipe23[0]);
  close(pipe23[1]);
  int st1 = 0, st2 = 0, st3 = 0;
  waitpid(p1, &st1, 0);
  waitpid(p2, &st2, 0);
  waitpid(p3, &st3, 0);
  int rc = 0;
  if (WIFEXITED(st1)) {
    if (WEXITSTATUS(st1) != 0) {
      fprintf(stderr, "p1 exited with status %d\n", WEXITSTATUS(st1));
      rc = 1;
    }
  } else {
    fprintf(stderr, "p1 terminated by signal %d\n", WTERMSIG(st1));
    rc = 1;
  }
  if (WIFEXITED(st2)) {
    if (WEXITSTATUS(st2) != 0) {
      fprintf(stderr, "p2 exited with status %d\n", WEXITSTATUS(st2));
      rc = 1;
    }
  } else {
    fprintf(stderr, "p2 terminated by signal %d\n", WTERMSIG(st2));
    rc = 1;
  }
  if (WIFEXITED(st3)) {
    if (WEXITSTATUS(st3) != 0) {
      fprintf(stderr, "p3 exited with status %d\n", WEXITSTATUS(st3));
      rc = 1;
    }
  } else {
    fprintf(stderr, "p3 terminated by signal %d\n", WTERMSIG(st3));
    rc = 1;
  }


  return 0;
}
