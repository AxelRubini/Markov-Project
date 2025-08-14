#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include "CSV_line_parser.h"
#include "next_word.h"
#include "linked_list.h"
#include "utf8_tools.h"
#include "utils.h"

// Helper: iterate linked_list_t assuming it stores next_word_t*
static size_t list_count_and_sum(linked_list_t *lst, double *psum) {
    *psum = 0.0;
    size_t n = 0;
    for (ll_item_t *it = lst->head; it != NULL; it = it->next) {
        next_word_t *nw = (next_word_t*)it->data;
        *psum += nw->probability;
        n++;
    }
    return n;
}

// Helper: check presence of a word with (approx) probability
static int has_word_with_prob(linked_list_t *lst, const int *word, double p, double eps) {
    for (ll_item_t *it = lst->head; it != NULL; it = it->next) {
        next_word_t *nw = (next_word_t*)it->data;
        if (utf8_strcmp(nw->word, word) == 0 && fabs(nw->probability - p) <= eps) {
            return 1;
        }
    }
    return 0;
}

static void test_one_line(const char *line,
                          const int **exp_words, const double *exp_probs, size_t exp_n) {
    int key[MAX_WORD_LENGTH] = {0};
    size_t offset = get_key_from_line(line, key);
    if (offset == (size_t)-1) {
        fprintf(stderr, "get_key_from_line failed for: %s\n", line);
        exit(1);
    }

    linked_list_t *lst = get_next_words_from_line(line, &offset);
    if (!lst) {
        fprintf(stderr, "get_next_words_from_line failed for: %s\n", line);
        exit(2);
    }

    double sum = 0.0;
    size_t n = list_count_and_sum(lst, &sum);
    if (n != exp_n) {
        fprintf(stderr, "Expected %zu items, got %zu for: %s\n", exp_n, n, line);
        exit(3);
    }
    if (sum > 1.0 + 1e-9) {
        fprintf(stderr, "Sum of probabilities > 1.0 (%.12f) for: %s\n", sum, line);
        exit(4);
    }

    for (size_t i = 0; i < exp_n; ++i) {
        if (!has_word_with_prob(lst, exp_words[i], exp_probs[i], 1e-6)) {
            fprintf(stderr, "Missing expected (word,prob) at i=%zu for: %s\n", i, line);
            exit(5);
        }
    }

    free_linked_list(lst);
}

int main(void) {
    // Example 1 (from the spec): "tempo,incerto,0.3333,di,0.3333,?,0.3333\n"
    const char *line1 = "tempo,incerto,0.3333,di,0.3333,?,0.3333\n";
    int w_incerto[] = {'i','n','c','e','r','t','o',0};
    int w_di[]      = {'d','i',0};
    int w_qmark[]   = {'?',0};
    const int *exp_words1[] = { w_incerto, w_di, w_qmark };
    const double exp_probs1[] = { 0.3333, 0.3333, 0.3333 };
    test_one_line(line1, exp_words1, exp_probs1, 3);

    // Example 2: "oggi,tempo,1\n"
    const char *line2 = "oggi,tempo,1\n";
    int w_tempo[] = {'t','e','m','p','o',0};
    const int *exp_words2[] = { w_tempo };
    const double exp_probs2[] = { 1.0 };
    test_one_line(line2, exp_words2, exp_probs2, 1);

    // Example 3 (spazi e newline finale): "key1 , a , 0.5 , b , 0.5 \n"
    const char *line3 = "key1 , a , 0.5 , b , 0.5 \n";
    int w_a[] = {'a',0};
    int w_b[] = {'b',0};
    const int *exp_words3[] = { w_a, w_b };
    const double exp_probs3[] = { 0.5, 0.5 };
    test_one_line(line3, exp_words3, exp_probs3, 2);

    puts("OK: CSV_line_parser basic cases passed.");
    return 0;
}
