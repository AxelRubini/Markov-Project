#include <assert.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <time.h>
#include <errno.h>
#include "../include/CSV_reader.h"
#include "../include/utils.h"




// ----- small utility: write whole buffer to fd (handle partial writes) -----
static void xwrite_all(int fd, const char *s) {
    size_t n = strlen(s);
    while (n > 0) {
        ssize_t w = write(fd, s, n);
        if (w < 0) { perror("write"); exit(1); }
        n -= (size_t)w;
        s += w;
    }
}

// ----- generate a unique filename into 'out', then try to open it safely -----
static int open_unique_temp(char *out, size_t out_sz) {
    // We try several candidates: "test_tmp_<timestamp>_<clock>_<counter>.tmp"
    // open(..., O_CREAT|O_EXCL) ensures we never clobber an existing file.
    unsigned long seed = (unsigned long)time(NULL) ^ (unsigned long)clock();
    int attempts = 128;

    for (int i = 0; i < attempts; ++i) {
        // Simple variability: add i and a rand() based on seed
        srand((unsigned)seed + (unsigned)i);
        unsigned r = (unsigned)rand();

        int n = snprintf(out, out_sz, "test_tmp_%lu_%u_%03d.tmp",
                         (unsigned long)time(NULL), r, i);
        if (n <= 0 || (size_t)n >= out_sz) {
            // Name truncated or snprintf error; try next iteration.
            continue;
        }

        int fd = open(out, O_CREAT | O_EXCL | O_RDWR, 0600);
        if (fd >= 0) return fd;     // success!
        if (errno != EEXIST) {
            // Any error different from "already exists" is fatal for us
            perror("open");
            return -1;
        }
        // If EEXIST, loop and try a new candidate name
    }
    fprintf(stderr, "Failed to create a unique temporary file after many attempts\n");
    return -1;
}

int main(void) {
    // 1) Create a fresh unique temp file (without mkstemp)
    char path[128];
    int fd = open_unique_temp(path, sizeof(path));
    if (fd < 0) return 1;

    // 2) Fill it with 4 records; the last one WITHOUT '\n' to test EOF behavior.
    xwrite_all(fd, "key1,a,0.5,b,0.5\n");
    xwrite_all(fd, "tempo,incerto,0.3333,di,0.3333,?,0.3333\n");
    xwrite_all(fd, "oggi,tempo,1\n");
    xwrite_all(fd, "ultimo,record,senzanewline,1"); // no trailing newline

    // 3) Rewind for reading
    if (lseek(fd, 0, SEEK_SET) < 0) { perror("lseek"); close(fd); remove(path); return 2; }

    // 4) Prepare buffer and capacity for read_csv_line()
    char *buf = NULL;
    size_t cap = 0;

    // 5) Expected lines
    const char *exp[] = {
        "key1,a,0.5,b,0.5\n",
        "tempo,incerto,0.3333,di,0.3333,?,0.3333\n",
        "oggi,tempo,1\n",
        "ultimo,record,senzanewline,1"
    };
    const size_t expN = sizeof(exp)/sizeof(exp[0]);

    // 6) Read loop + checks
    for (size_t i = 0; i < expN; ++i) {
        ssize_t r = read_csv_line(fd, &buf, &cap);
        if (r < 0) {
            fprintf(stderr, "read_csv_line returned error at line %zu\n", i);
            free(buf);
            close(fd);
            remove(path);
            return 3;
        }
        if (r == 0) {
            fprintf(stderr, "Unexpected EOF at line %zu\n", i);
            free(buf);
            close(fd);
            remove(path);
            return 4;
        }
        if ((size_t)r != strlen(exp[i]) || strncmp(buf, exp[i], (size_t)r) != 0) {
            fprintf(stderr, "Mismatch at line %zu\nExpected: [%s]\nGot:      [%.*s]\n",
                    i, exp[i], (int)r, buf);
            free(buf);
            close(fd);
            remove(path);
            return 5;
        }
    }

    // 7) Next call must return 0 (clean EOF)
    ssize_t r = read_csv_line(fd, &buf, &cap);
    if (r != 0) {
        fprintf(stderr, "Expected EOF (0), got %zd\n", r);
        free(buf);
        close(fd);
        remove(path);
        return 6;
    }

    // 8) Cleanup
    free(buf);
    close(fd);
    // Use C standard remove() after closing (portable), instead of unlink while open
    if (remove(path) != 0) perror("remove");

    puts("OK: read_csv_line basic/EOF cases passed.");
    return 0;
}

