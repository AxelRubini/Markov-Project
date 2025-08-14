#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

#include "../include/file_reader_mp.h"   
#include "../include/CSV_writer.h"       
#include "../include/hash_table.h"       
#include "../include/word.h"

static void die(const char *msg) {
    perror(msg);
    _exit(1);
}

int main(int argc, char **argv) {
    const char *in_path = "test_files/test_reader_utf8.txt"; // Default input path
    const char *out_path = "test_files/out_test_csv_writer_mp.csv"; // Default output path

    // Due pipe: p1 -> p2 (pipe12) e p2 -> p3 (pipe23)
    int pipe12[2];
    int pipe23[2];
    if (pipe(pipe12) == -1) die("pipe12");
    if (pipe(pipe23) == -1) die("pipe23");

    // --- p3 ---
    pid_t p3 = fork();
    if (p3 == -1) die("fork p3");
    if (p3 == 0) {
        // Child p3: legge CSV line da pipe23[0] e scrive su out_path
        // Chiude estremi inutili
        close(pipe12[0]);
        close(pipe12[1]);
        close(pipe23[1]);

        // Riceve e scrive
        if (csv_line_recv(pipe23[0], out_path) < 0) {
            fprintf(stderr, "p3: csv_line_recv fallita\n");
            close(pipe23[0]);
            _exit(1);
        }

        close(pipe23[0]);
        _exit(0);
    }

    // --- p2 ---
    pid_t p2 = fork();
    if (p2 == -1) die("fork p2");
    if (p2 == 0) {
        // Child p2: legge coppie di parole da pipe12[0], le inserisce in HT,
        // poi invia le righe CSV su pipe23[1]
        close(pipe12[1]);      // scrittura non usata
        close(pipe23[0]);      // lettura non usata

        // Crea hash table (adatta alla tua API: ht_create, ht_new, ecc.)
        // Se la tua funzione differisce, cambia la riga seguente.
        hash_table_t *ht = create_hash_table(HASH_TABLE_SIZE, word_hash, word_hashtable_keycmp);  // <— dimensione a scelta
        if (!ht) {
            fprintf(stderr, "p2: impossibile creare hash table\n");
            _exit(1);
        }

        if (read_words_from_pipe(pipe12[0], ht) < 0) {
            fprintf(stderr, "p2: read_words_from_pipe fallita\n");
            close(pipe12[0]);
            close(pipe23[1]);
            _exit(1);
        }
        close(pipe12[0]); // finita la lettura

        if (csv_line_send(pipe23[1], ht) < 0) {
            fprintf(stderr, "p2: csv_line_send fallita\n");
            close(pipe23[1]);
            _exit(1);
        }
        close(pipe23[1]);

        _exit(0);
    }

    // --- p1 ---
    pid_t p1 = fork();
    if (p1 == -1) die("fork p1");
    if (p1 == 0) {
        // Child p1: apre il file di input e pubblica le coppie di parole su pipe12[1]
        close(pipe12[0]);
        close(pipe23[0]);
        close(pipe23[1]);

        int fd_in = open(in_path, O_RDONLY);
        if (fd_in < 0) {
            perror("p1: open input");
            close(pipe12[1]);
            _exit(1);
        }

        if (publish_word_on_pipe(fd_in, pipe12[1]) < 0) {
            fprintf(stderr, "p1: publish_word_on_pipe fallita\n");
            close(fd_in);
            close(pipe12[1]);
            _exit(1);
        }

        close(fd_in);
        close(pipe12[1]); // segnala EOF a p2
        _exit(0);
    }

    // Processo padre: chiude tutte le estremità che non usa e attende
    close(pipe12[0]);
    close(pipe12[1]);
    close(pipe23[0]);
    close(pipe23[1]);

    int st1 = 0, st2 = 0, st3 = 0;
    waitpid(p1, &st1, 0);
    waitpid(p2, &st2, 0);
    waitpid(p3, &st3, 0);

    int rc = 0;
    if (!WIFEXITED(st1) || WEXITSTATUS(st1) != 0) rc = 1;
    if (!WIFEXITED(st2) || WEXITSTATUS(st2) != 0) rc = 1;
    if (!WIFEXITED(st3) || WEXITSTATUS(st3) != 0) rc = 1;

    if (rc == 0) {
        fprintf(stdout, "OK: pipeline p1->p2->p3 completata. Output: %s\n", out_path);
    } else {
        fprintf(stderr, "ERRORE: almeno uno dei processi ha fallito.\n");
    }

    return rc;
}
