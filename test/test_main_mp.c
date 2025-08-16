// test_main_sp_mp.c  (sostituisci il tuo file di test con questo)

// Standard C
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>     // access, unlink
#include <sys/wait.h>   // wait

// Progetto
#include "options.h"
#include "runner.h"

// Macro utile per contare gli elementi NON NULL terminatori
#define ARGV_COUNT_WITH_NULL(a)   ((int)((sizeof(a)/sizeof((a)[0])) - 1))
#define SAFE_STR(s)               ((s) ? (s) : "NULL")

static int is_interactive(void) {
    const char *env = getenv("INTERACTIVE");
    return (env && strcmp(env, "1") == 0);
}

// ------------------------------------------------------------
// Utility: creazione file di test
// ------------------------------------------------------------
void create_test_file(const char *filename, const char *content) {
    FILE *file = fopen(filename, "w");
    if (!file) {
        fprintf(stderr, "Errore nella creazione del file di test: %s\n", filename);
        return;
    }
    fprintf(file, "%s", content);
    fclose(file);
    printf("File di test creato: %s\n", filename);
}

void create_test_csv(const char *filename) {
    FILE *file = fopen(filename, "w");
    if (!file) {
        fprintf(stderr, "Errore nella creazione del file CSV di test: %s\n", filename);
        return;
    }
    // Header + righe
    fprintf(file, "word,frequency\n");
    fprintf(file, "the,150\n");
    fprintf(file, "and,120\n");
    fprintf(file, "of,100\n");
    fprintf(file, "to,80\n");
    fprintf(file, "in,75\n");
    fprintf(file, "is,60\n");
    fprintf(file, "that,55\n");
    fprintf(file, "for,50\n");
    fprintf(file, "with,45\n");
    fprintf(file, "on,40\n");
    fclose(file);
    printf("File CSV di test creato: %s\n", filename);
}

// ------------------------------------------------------------
// Esegue UN test
// ------------------------------------------------------------
int test_configuration(const char *test_name, char **argv, int argc) {
    printf("\n============================================================\n");
    printf("TEST: %s\n", SAFE_STR(test_name));
    printf("============================================================\n");

    // Debug: stampa argomenti in modo sicuro
    printf("Argomenti passati:\n");
    for (int i = 0; i < argc; i++) {
        if (argv[i] == NULL) {
            printf("  argv[%d]: (NULL)  <-- BUG NEI TEST: argc=%d\n", i, argc);
            // Evitiamo di andare oltre
            argc = i; // tronca per sicurezza
            break;
        }
        printf("  argv[%d]: %s\n", i, argv[i]);
    }
    printf("\n");

    fflush(stdout);
    fflush(stderr);

    options_t options;
    printf("Parsing opzioni...\n");
    fflush(stdout);

    int result = parse_options(argc, argv, &options);
    if (result != 0) {
        printf("❌ Errore nel parsing delle opzioni per il test: %s\n", SAFE_STR(test_name));
        printf("   Codice di errore: %d\n", result);
        fflush(stdout);
        return -1;
    }

    printf("✅ Opzioni parsate correttamente\n");
    fflush(stdout);

    // Verifica file input e crea se manca il CSV
    if (options.input_txt && access(options.input_txt, F_OK) != 0) {
        printf("⚠️ File di input TXT non trovato: %s\n", options.input_txt);
    }
    if (options.input_csv && access(options.input_csv, F_OK) != 0) {
        printf("⚠️ File di input CSV non trovato: %s\n", options.input_csv);
        printf("   Creazione del file CSV mancante...\n");
        create_test_csv(options.input_csv);
    }

    // Debug: stampa opzioni
    printf("Opzioni parsate:\n");
    printf("  Mode: %s\n", options.mode == 0 ? "SP" : "MP");
    printf("  Task: %d\n", options.task);
    printf("  Input TXT: %s\n", SAFE_STR(options.input_txt));
    printf("  Input CSV: %s\n", SAFE_STR(options.input_csv));
    printf("  Output TXT: %s\n", SAFE_STR(options.output_txt));
    printf("  Output CSV: %s\n", SAFE_STR(options.output_csv));
    printf("  N Words: %zu\n", options.n_words);
    printf("  Word: %s\n", SAFE_STR(options.word));
    fflush(stdout);

    // Esegui
    printf("Avvio esecuzione...\n");
    fflush(stdout);
    result = run(&options);

    printf("Esecuzione terminata con codice: %d\n", result);
    if (result == 0) {
        printf("✅ Test completato con successo: %s\n", SAFE_STR(test_name));
    } else {
        printf("❌ Test fallito: %s (codice: %d)\n", SAFE_STR(test_name), result);
    }
    fflush(stdout);
    return result;
}

// ------------------------------------------------------------
// Pulizia file
// ------------------------------------------------------------
void cleanup_test_files(void) {
    printf("\nPulizia file di test...\n");
    const char *files[] = {
        "test_input.txt",
        "test_input.csv",
        "test_output.txt",
        "test_output.csv",
        "Analysis_test_input.txt",
        "Analysis_STDIN_text.csv"
    };
    for (size_t i = 0; i < sizeof(files)/sizeof(files[0]); i++) {
        if (unlink(files[i]) == 0) {
            printf("Rimosso: %s\n", files[i]);
        }
    }
}

// ------------------------------------------------------------
// Help
// ------------------------------------------------------------
void show_help(void) {
    printf("\n============================================================\n");
    printf("HELP - Opzioni disponibili:\n");
    printf("============================================================\n");
    printf("Modalità di esecuzione:\n");
    printf("  --sp              Single Process mode (default)\n");
    printf("  --mp              Multi Process mode\n\n");

    printf("Task da eseguire:\n");
    printf("  --task1           Esegui solo Task 1\n");
    printf("  --task2           Esegui solo Task 2\n");
    printf("  --task-both       Esegui entrambi i task (default)\n\n");

    printf("File di input/output:\n");
    printf("  --input-txt FILE  File di testo in input\n");
    printf("  --input-csv FILE  File CSV in input\n");
    printf("  --output-txt FILE File di testo in output\n");
    printf("  --output-csv FILE File CSV in output\n\n");

    printf("Parametri aggiuntivi:\n");
    printf("  --n-words N       Numero di parole da processare\n");
    printf("  --word WORD       Parola specifica da processare\n\n");
}

// ------------------------------------------------------------
// main
// ------------------------------------------------------------
int main(int argc, char *argv[]) {
    printf("============================================================\n");
    printf("SISTEMA DI TEST COMPLETO PER IL PROGETTO\n");
    printf("============================================================\n");

    // Modalità "pass-through" dalla command line
    if (argc > 1) {
        if (strcmp(argv[1], "--help") == 0 || strcmp(argv[1], "-h") == 0) {
            show_help();
            return 0;
        }
        printf("Esecuzione con parametri dalla command line...\n");
        options_t options;
        int result = parse_options(argc, argv, &options);
        if (result != 0) {
            printf("❌ Errore nel parsing delle opzioni\n");
            return -1;
        }
        return run(&options);
    }

    // Dati di test
    printf("Creazione file di test...\n");
    const char *test_text =
        "This is a sample text for testing purposes. "
        "The text contains multiple words and sentences. "
        "We will use this text to test the word analysis functionality. "
        "The program should be able to count word frequencies and generate reports. "
        "This is just a simple example with common English words like the, and, of, to, in, is, that, for, with, on.";

    create_test_file("test_input.txt", test_text);
    create_test_csv("test_input.csv");

    // NOTA: puoi cambiare i path CSV qui sotto se vuoi usare altri file:
    static char csv_task2_path1[] = "test_input.csv";
    static char csv_task2_path2[] = "test_input.csv";

    // Definizione ARG per ogni test (NULL-terminated)
    static const char *t_sp_task1[] = {
        "main", "--sp", "--task1",
        "--input-txt", "test_input.txt",
        "--output-csv", "test_output.csv",
        NULL
    };
    static const char *t_sp_task2[] = {
        "main", "--sp", "--task2",
        "--input-csv", "test_files/out_test_csv_writer_mp.csv",
        "--output-txt", "test_output.txt",
        "--n-words", "5",
        "--word", "the",
        NULL
    };
    static const char *t_sp_both[] = {
        "main", "--sp", "--task-both",
        "--input-txt", "test_input.txt",
        "--output-csv", "test_output.csv",
        "--output-txt", "test_output.txt",
        "--n-words", "5",
        NULL
    };
    static const char *t_mp_task1[] = {
        "main", "--mp", "--task1",
        "--input-txt", "test_input.txt",
        "--output-csv", "test_output.csv",
        NULL
    };
    static const char *t_mp_task2[] = {
        "main", "--mp", "--task2",
        "--input-csv", "test_files/out_test_csv_writer_mp.csv",
        "--output-txt", "test_output.txt",
        "--n-words", "3",
        NULL
    };
    static const char *t_mp_both[] = {
        "main", "--mp", "--task-both",
        "--input-txt", "test_input.txt",
        "--output-csv", "test_output.csv",
        "--output-txt", "test_output.txt",
        "--n-words", "7",
        "--word", "test",
        NULL
    };

    // Tabella dei test
    struct test_case {
        const char      *name;
        char           **argv;
        int              argc;
    } tests[] = {
        { "Task 1 - Single Process", (char **)t_sp_task1, ARGV_COUNT_WITH_NULL(t_sp_task1) },
        { "Task 2 - Single Process", (char **)t_sp_task2, ARGV_COUNT_WITH_NULL(t_sp_task2) },
        { "Both Tasks - Single Process", (char **)t_sp_both, ARGV_COUNT_WITH_NULL(t_sp_both) },
        { "Task 1 - Multi Process", (char **)t_mp_task1, ARGV_COUNT_WITH_NULL(t_mp_task1) },
        { "Task 2 - Multi Process", (char **)t_mp_task2, ARGV_COUNT_WITH_NULL(t_mp_task2) },
        { "Both Tasks - Multi Process", (char **)t_mp_both, ARGV_COUNT_WITH_NULL(t_mp_both) },
    };

    int total_tests  = (int)(sizeof(tests)/sizeof(tests[0]));
    int passed_tests = 0;

    for (int i = 0; i < total_tests; i++) {
        if (test_configuration(tests[i].name, tests[i].argv, tests[i].argc) == 0) {
            passed_tests++;
        }

        if (is_interactive()) {
            printf("\nPremere ENTER per continuare al prossimo test...");
            (void)getchar();
        }
    }

    // Risultati
    printf("\n============================================================\n");
    printf("RISULTATI FINALI DEI TEST\n");
    printf("============================================================\n");
    printf("Test eseguiti: %d\n", total_tests);
    printf("Test passati: %d\n", passed_tests);
    printf("Test falliti: %d\n", total_tests - passed_tests);

    if (passed_tests == total_tests) {
        printf("🎉 TUTTI I TEST SONO PASSATI! 🎉\n");
    } else {
        printf("⚠️ Alcuni test sono falliti. Controlla l'implementazione.\n");
    }

    if (is_interactive()) {
        printf("\nVuoi rimuovere i file di test? (y/n): ");
        char choice = 'n';
        if (scanf(" %c", &choice) == 1) {
            if (choice == 'y' || choice == 'Y') {
                cleanup_test_files();
            }
        }
    }

    return (passed_tests == total_tests) ? 0 : 1;
}
