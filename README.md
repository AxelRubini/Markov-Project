# Markov

Il programma analizza un testo di input, costruisce le probabilità di transizione tra parole e produce nuovo testo partendo da queste statistiche.

## Struttura del progetto

- **Makefile** – script di compilazione principale; genera l'eseguibile `markov`.
- **compile_flags.txt** – opzioni di compilazione utilizzate dagli editor.
- **lib/argtable3/** – libreria statica per il parsing della riga di comando.
- **include/** – file header suddivisi per sottosistemi:
  - `app/` – dichiarazioni per il parser degli argomenti e il runner.
    - `options.h` - definizione della struttura `options_t` per le opzioni di esecuzione.
    - `runner.h` - dichiarazione della funzione `run` per avviare la task selezionata.
  - `core/` – funzioni di utilità generiche.
    - `next_word_chooser.h` - dichiarazione della struct `next_word_chooser_t` e della funzione per la scelta della parola successive.
    - `utf8_tools.h` - funzioni per la gestione di stringhe UTF-8.
    - `utils.h` - funzioni generiche di utilita e wrapper funzioni di sistema.
  - `csv/` – gestione della lettura e scrittura CSV.
    - `CSV_line_builder.h` - funzioni per costruire le stringhe csv dalla tabella hash.
    - `CSV_line_parser.h` - funzioni per il parsing delle righe CSV.
    - `CSV_reader.h` - funzioni per la lettura di file CSV sia in singolo che multiprocesso.
    - `CSV_writer.h` - funzioni per la scrittura di file CSV sia in singolo che multiprocesso.
  - `ds/` – strutture dati (linked list,hash table, ecc.).
    - `hash_table.h` - dichiarazione della struttura `hash_table_t` e delle funzioni per la gestione della tabella hash.
    - `linked_list.h` - dichiarazione delle strutture `linked_list_t` e `ll_item_t` per la gestione delle linked list e relative funzioni per la gestione di esse.
    - `ht_item.h` - dichiarazione della struttura `ht_item` e delle funzioni per la gestione degli elementi della tabella hash, il suo scopo e' quello di fornire un' interfaccia generica per oggetti piu' specializzati come word e next\*word.
      - `word.h` - dichiarazione della struttura `word_t` e delle funzioni per gestire la struct e funzioni per la specializzazione della `hash_table_t` per le parole e i relativi `ht_item`.
        un singolo elemento della hash table che rappresenta una parola e le sue occorrenze piu' le parole successive e le occorrenze come tali rispetto alla parola chiave avrà la seguente struttura:

    ```markdown
    word_t {
    int \*word;
    int occurrences;
    }
    ht_item {
    word_t \*key;
    linked_list_t \*next_words; // lista di word_t successive
    }
    ```

    - `next_word.h` - dichiarazione della struttura `next_word_t` e delle funzioni per gestire la struct e funzioni per la specializzazione della `hash_table_t` per le parole successive e i relativi `ht_item`.
      Un singolo elemento della hash table che utilizza la struct `next_word_t` rappresenta con una lista di code point la parola chiave e le parole successive useando la struct `next_word_t` che contiene la probabilità di transizione dalla parola chiave alla parola successiva creando quindi una struttura:

    ```markdown
    next_word_t {
    int \*next_word;
    double probability;
    }
    ht_item {
    int \*key; // parola chiave
    linked_list_t \*next_words; // lista di next_word_t successive
    }
    ```

  - `io/` – funzioni di input/output su file e pipe.
    - `file_reader.h` - contiene le funzione per la lettura di un testo UTF-8 e la creazione di una hash table di parole e leloro occorrenze sfruttando la struct `word_t`.
    - `file_reader_mp.h` - contiene le funzioni per la lettura di un testo UTF-8 e la creazione di una hash table di parole e le loro occorrenze sfruttando la struct `word_t` le funzioni presenti sono pensate per essere utilizzate in modalità multipocesso.
    - `file_writer.h` - contiene le funzione per la scrittura di un testo UTF-8 a partire da una hash table di parole e le probabilità di transizione tra di esse, sfruttando la struct `next_word_t` il file contiene sia le funzioni utilizzate in modaità singolo processo che quelle utilizzate in modalità multiprocesso.
    - `frame.h` - contiene le funzioni per la gestione delle pipe e della comunicazione tra processi.

  - `modes/` – interfacce per le modalità _single process_ (`sp`) e _multi process_ (`mp`).
    - `sp/` - implementazione delle modalità singolo processo.
      - `task1.h` - dichiarazione della funzione `task1` per l' analisi di un testo e la generazione di un file CSV.
      - `task2.h` - dichiarazione della funzione `task2` per la generazione di un testo a partire da un CSV.
        \_ `task_both.h` - dichiarazione della funzione `task_both` per l' esecuzione sequenziale di `task1` e `task2`.
    - `mp/` - implementazione delle modalità multiprocesso.
      - `task1.h` - dichiarazione della funzione `task1_mp` per l' analisi di un testo e la generazione di un file CSV.
      - `task2.h` - dichiarazione della funzione `task2_mp` per la generazione di un testo a partire da un CSV.
      - `task_both.h` - dichiarazione della funzione `task_both_mp` per l' esecuzione di `task1_mp` e `task2_mp`.
    - `modes_utils.h` - funzioni comuni per le modalità `sp` e `mp`.

- **src/** – sorgenti C organizzati come in `include/`:
  - `app/` – `main.c`, parser delle opzioni e lancio delle attività.
  - `modes/sp/` e `modes/mp/` – implementazioni delle attività in modalità a processo singolo o multiplo.
  - `core/`, `csv/`, `ds/`, `io/` – componenti di supporto.

## Compilazione

Assicurarsi di avere un compilatore GCC. Dal root del progetto eseguire:

```bash
make
```

L'eseguibile risultante sarà `./markov`.

## Manuale utente

Esecuzione generale:

```bash
./markov [opzioni]
```

Opzioni principali:

- `--sp` / `--mp` – esegue rispettivamente in modalità singolo processo o multiprocesso.
- `--task1` – genera un file CSV analizzando un testo.
- `--task2` – genera nuovo testo a partire da un CSV.
- `--task-both` – esegue `task1` seguito da `task2`.
- `--input-txt <file>` – testo di input per `task1`(default: stdin).
- `--input-csv <file>` – CSV di input per `task2`(obbligatorio se `task2` è eseguita da sola).
- `--output-txt <file>` – file di testo prodotto da `task2` (default: stdout).
- `--output-csv <file>` – file CSV prodotto da `task1` (default: `Analysis_<input>`).
- `--n-words <n>` – numero di parole da generare (default: 10).
  Durante l'esecuzione di `task2` o `task-both` verrà richiesto di inserire inserire una parola o parte di esse, il programma quindi stampera' a schermo tutte le parole che contengono l' input come sotto stringa, se non si vuole cercare alcuna parola premere exit, se viene eseguita una ricerca ma non si vuole scegliere tra le parole proposte premere invio perricominciare la ricerca oppure uscire, sen non verrà scelta alcuna parola uno starter randomico tra quelli seguenti verra selezionato '.' '?' '!'.

### Esempi

1. Analizzare un testo e salvare le transizioni in CSV:

   ```bash
   ./markov --sp --task1 --input-txt libro.txt --output-csv analisi.csv
   ```

2. Generare 20 parole a partire da un CSV esistente:

   ```bash
   ./markov --mp --task2 --input-csv analisi.csv --output-txt output.txt --n-words 20
   ```

   Durante l'esecuzione verrà richiesta la parola iniziale; premere Invio per una scelta casuale tra '.' '?' '!'.

3. Eseguire entrambe le attività in sequenza:

   ```bash
   ./markov --mp --task-both --input-txt libro.txt --output-csv analisi.csv --output-txt output.txt --n-words 20

   ```

`NOTA`: Il carattere tipografico apostrofo chiuso a destra viene considerato tale se e solo se no e' presente alcuno spazio successivo adesso, altrimenti viene considerato come una chiusura di single quote e quindi ignorato

## Flowchart delle modalita multiprocesso

### Task-both process flow

\dot
digraph Pipeline {
rankdir=LR; splines=true; nodesep=0.5; ranksep=0.6;
node [shape=box, style="rounded"];

I [label="input_txt / stdin", shape=oval, style=""];
P1 [label="P1: w_p1"];
P2 [label="P2: w_p2\nHT word→freq"];
P2a [label="P2_1: w_p2_1"];
P2b [label="P2_2: w_p2_2"];
P3 [label="P3: w_p3"];
P4 [label="P4: w_p4\nHT key→next prob"];
P5 [label="P5: w_p5"];
CSV [label="output_csv", shape=oval, style=""];
TXT [label="output_txt", shape=oval, style=""];
OUT [label="stdout", shape=oval, style=""];

I -> P1;
P1 -> P2 [label="pipe_read"];
P2 -> P2a [label="fork"];
P2 -> P2b [label="fork"];
P2a -> P3 [label="pipe_csv"];
P3 -> CSV [label="write"];
P2b -> P4 [label="pipe_freqtab"];
P4 -> P5 [label="pipe_out"];
P5 -> TXT [label="write"];
TXT -> OUT [style=dashed, label="if empty"];
}
\enddot

### Task1 process flow

\dot
digraph Task1 {
rankdir=LR; nodesep=0.6; ranksep=0.7; splines=true;
node [shape=box, style="rounded"];

I [label="input_txt", shape=oval, style=""];
P1 [label="P1: w_p1\nopen → publish_word_on_pipe"];
P2 [label="P2: w_p2\nread_words → HT → csv_line_send"];
P3 [label="P3: w_p3\ncsv_line_recv → write"];
CSV [label="output_csv", shape=oval, style=""];

I -> P1;
P1 -> P2 [label="pipe12"];
P2 -> P3 [label="pipe23"];
P3 -> CSV;
}
\enddot

### Task2 process flow

\dot
digraph Task2 {
rankdir=LR; nodesep=0.6; ranksep=0.7; splines=true;
node [shape=box, style="rounded"];

CSV [label="input_csv", shape=oval, style=""];
P1 [label="P1: w_p1\nopen → publish_line_on_pipe → close"];
P2 [label="P2: w_p2\nread_lines → HT → send_output_mp(n_words)"];
P3 [label="P3: w_p3\nwrite_output_mp(output_txt)"];
TXT [label="output_txt", shape=oval, style=""];
OUT [label="stdout (se output_txt empty)", shape=oval, style=""];

CSV -> P1;
P1 -> P2 [label="pipe12"];
P2 -> P3 [label="pipe23"];
P3 -> TXT;
TXT -> OUT [style=dashed];
}
\enddot

## Risorse usate per la realizzazione del progetto

### Analisi del testo in UTF-8 :

- https://en.wikipedia.org/wiki/UTF-8
- https://www.youtube.com/watch?v=c_hfKgektt4
- https://stackoverflow.com/questions/68835028/utf-8-hex-to-unicode-code-point-only-math
- https://utf8everywhere.org/

### Gestione IPC

- https://stackoverflow.com/questions/19127398/socket-programming-read-is-reading-all-of-my-writes
- https://people.cs.rutgers.edu/~pxk/416/notes/c-tutorials/pipe.html
  ho bisogno di implementare questa versione di scrittura per evitare problemi di buffer pieno
- https://stackoverflow.com/questions/8242318/trouble-when-trying-to-define-a-write-all-socket-function-in-c

### Hash table

- https://stackoverflow.com/questions/22437416/best-way-to-resize-a-hash-table
- https://blog.mischel.com/2008/10/16/optimizing-the-wrong-thing/
- https://dev.to/jihoonj/implementing-a-hash-table-3pa7
  risolve problema di resize
- https://gist.github.com/cellularmitosis/9db34df37d3df91709ad3e4faf93c417

### Funzione Hash

- http://www.cse.yorku.ca/~oz/hash.html
