//
// ----------------------------------------------------------------------
// ADAPTER: DICHIARAZIONI (allinea qui le firme alle tue funzioni reali)
#define _POSIX_C_SOURCE 200809L
#include "CSV_reader.h"
#include "file_writer.h"
#include <assert.h>
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <signal.h>


// ----------------------------------------------------------------------
// TEST LOGIC P1->P2->P3
// ----------------------------------------------------------------------

// Producer P1: read CSV and send each line with publish_line()
static int child_p1(int wfd, const char *csv_path) {
  if (wfd < 0 || !csv_path) {
    fprintf(stderr, "Invalid arguments to child_p1\n");
    return 1;
  }
  
  int fd = open(csv_path, O_RDONLY);
  if (fd < 0) {
    perror("open csv file");
    return 2;
  }
  
  int result = publish_line_on_pipe(fd, wfd);
  
    if (close(fd) < 0) {
      perror("close csv file");
      close(wfd);  // Close wfd on error
      return 3;
    }

    fprintf(stderr, "[P1] Finished writing to pipe12\n");
    close(wfd);  // Close wfd only once
    return result;  // Return result of publish_line_on_pipe
}

// Consumer/processor P2: read lines, build HT, then send chosen words to P3
static int child_p2(int rfd12, int wfd23, size_t max_items_to_send,
                    const char *start_symbol) {
  if (rfd12 < 0 || wfd23 < 0 || max_items_to_send == 0 || !start_symbol) {
    fprintf(stderr, "Invalid arguments to child_p2\n");
    return 1;
  }

    hash_table_t *ht = read_lines_from_pipe(rfd12);
    close(rfd12); // Close immediately after reading
  
  if (!ht) {
      fprintf(stderr, "read_lines_from_pipe failed\n");
      close(wfd23);  // Close wfd23 on error
      return 4;
  }
  
  if (ht->size == 0) {
      fprintf(stderr, "No lines read from pipe12\n");
      free_hash_table(ht);
      close(wfd23);  // Close wfd23 on error
      return 5;
  }

  fprintf(stderr, "[P2] HT size=%d\n", ht->size);

  int result = send_output_mp(wfd23, ht, max_items_to_send, start_symbol);
    free_hash_table(ht);  // Always free hash table
    close(wfd23);         // Always close wfd23
  
  if (result != 0) {
    fprintf(stderr, "send_output_mp failed\n");
    return 5;
  }
  
  return 0;
}

// Writer P3: receive words from pipe23 and write to output file.
static int child_p3(int rfd23, const char *out_path, size_t *out_count) {
  if (rfd23 < 0 || !out_path || !out_count) {
    fprintf(stderr, "Invalid arguments to child_p3\n");
    return 1;
  }
  
    int result = write_output_mp(rfd23, out_path);
    close(rfd23);  // Always close rfd23
  
  if (result != 0) {
    fprintf(stderr, "write_output_mp failed\n");
    return 2;
  }
  
  fprintf(stderr, "[P3] Output written to %s\n", out_path);
  return 0;
}

int main(int argc, char **argv) {
  (void)argc;
  (void)argv;
  const char *in_csv = "test_files/out_test_reader_utf8.csv";
  const char *out_txt = "test_files/out_test_writer_mp.txt";
  size_t max_items = 100;

  int pipe12[2], pipe23[2];
  if (pipe(pipe12) != 0) {
    perror("pipe12");
    return 1;
  }
  if (pipe(pipe23) != 0) {
    perror("pipe23");
    close(pipe12[0]);  // Pulisci pipe12 in caso di errore
    close(pipe12[1]);
    return 1;
  }

  pid_t p1 = fork();
  if (p1 < 0) {
    perror("fork p1");
    // Close all pipes on error
    close(pipe12[0]); close(pipe12[1]);
    close(pipe23[0]); close(pipe23[1]);
    return 1;
  }
  if (p1 == 0) {
    // P1: writer -> pipe12
    close(pipe12[0]); // read end unused
    close(pipe23[0]); // these pipes unused
    close(pipe23[1]);
    
    int rc = child_p1(pipe12[1], in_csv);
    _exit(rc);  // child_p1 already closes pipe12[1]
  }

  pid_t p2 = fork();
  if (p2 < 0) {
    perror("fork p2");
    // Terminate P1 and close pipes
    kill(p1, SIGTERM);
    close(pipe12[0]); close(pipe12[1]);
    close(pipe23[0]); close(pipe23[1]);
    return 1;
  }
  if (p2 == 0) {
    // P2: read pipe12, build HT, send to pipe23
    close(pipe12[1]); // write end unused on pipe12
    close(pipe23[0]); // read end unused on pipe23
    
    int rc = child_p2(pipe12[0], pipe23[1], max_items, ".");
    _exit(rc);  // child_p2 already closes its fds
  }

  pid_t p3 = fork();
  if (p3 < 0) {
    perror("fork p3");
    // Terminate P1 and P2, close pipes
    kill(p1, SIGTERM);
    kill(p2, SIGTERM);
    close(pipe12[0]); close(pipe12[1]);
    close(pipe23[0]); close(pipe23[1]);
    return 1;
  }
  if (p3 == 0) {
    // P3: read pipe23 and write output file
    close(pipe12[0]); // pipe12 unused
    close(pipe12[1]);
    close(pipe23[1]); // write end unused on pipe23
    
    size_t out_count = 0;
    int rc = child_p3(pipe23[0], out_txt, &out_count);
    _exit(rc);  // child_p3 already closes pipe23[0]
  }

  // Parent process: close ALL pipes
  // CRUCIAL: parent must close all to avoid deadlock
  close(pipe12[0]);
  close(pipe12[1]);
  close(pipe23[0]);
  close(pipe23[1]);

  int st1 = 0, st2 = 0, st3 = 0;
  waitpid(p1, &st1, 0);
  waitpid(p2, &st2, 0);
  waitpid(p3, &st3, 0);

  if (WIFEXITED(st1) && WEXITSTATUS(st1) == 0 && 
      WIFEXITED(st2) && WEXITSTATUS(st2) == 0 && 
      WIFEXITED(st3) && WEXITSTATUS(st3) == 0) {
    printf("OK: test multi-processo P1->P2->P3 passato.\n");
    return 0;
  }

  // More detailed error diagnostics
  if (!WIFEXITED(st1))
    fprintf(stderr, "P1 terminato da segnale %d\n", WTERMSIG(st1));
  else if (WEXITSTATUS(st1) != 0)
    fprintf(stderr, "P1 exit=%d\n", WEXITSTATUS(st1));

  if (!WIFEXITED(st2))
    fprintf(stderr, "P2 terminato da segnale %d\n", WTERMSIG(st2));
  else if (WEXITSTATUS(st2) != 0)
    fprintf(stderr, "P2 exit=%d\n", WEXITSTATUS(st2));

  if (!WIFEXITED(st3))
    fprintf(stderr, "P3 terminato da segnale %d\n", WTERMSIG(st3));
  else if (WEXITSTATUS(st3) != 0)
    fprintf(stderr, "P3 exit=%d\n", WEXITSTATUS(st3));

  return 1;
}
