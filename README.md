# Markov

Il programma analizza un testo di input, costruisce le probabilità di transizione tra parole e può produrre nuovo testo partendo da queste statistiche.

## Struttura del progetto

- **Makefile** – script di compilazione principale; genera l'eseguibile `markov`.
- **compile_flags.txt** – opzioni di compilazione utilizzate dagli editor.
- **resources/** – risorse usate per la realizzazione del progetto.
- **lib/argtable3/** – libreria statica per il parsing della riga di comando.
- **include/** – file header suddivisi per sottosistemi:
  - `app/` – dichiarazioni per il parser degli argomenti e il runner.
  - `core/` – funzioni di utilità generiche.
  - `csv/` – gestione della lettura e scrittura CSV.
  - `ds/` – strutture dati (liste, tabelle hash, ecc.).
  - `io/` – funzioni di input/output su file e pipe.
  - `modes/` – interfacce per le modalità *single process* (`sp`) e *multi process* (`mp`).
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
- `--word <parola>` – parola iniziale; se omessa viene scelta casualmente.

### Esempi

1. Analizzare un testo e salvare le transizioni in CSV:

   ```bash
   ./markov --sp --task1 --input-txt libro.txt --output-csv analisi.csv
   ```

2. Generare 20 parole a partire da un CSV esistente, scegliendo "." come parola iniziale:

   ```bash
   ./markov --mp --task2 --input-csv analisi.csv --output-txt output.txt --n-words 20 --word . 
   ```


## Flowchart delle modalita multiprocesso

### Task-both process flow 
\dot
digraph Pipeline {
  rankdir=LR; splines=true; nodesep=0.5; ranksep=0.6;
  node [shape=box, style="rounded"];

  I   [label="input_txt / stdin", shape=oval, style=""];
  P1  [label="P1: w_p1"];
  P2  [label="P2: w_p2\nHT word→freq"];
  P2a [label="P2_1: w_p2_1"];
  P2b [label="P2_2: w_p2_2"];
  P3  [label="P3: w_p3"];
  P4  [label="P4: w_p4\nHT key→next prob"];
  P5  [label="P5: w_p5"];
  CSV [label="output_csv", shape=oval, style=""];
  TXT [label="output_txt", shape=oval, style=""];
  OUT [label="stdout", shape=oval, style=""];

  I  -> P1;
  P1 -> P2  [label="pipe_read"];
  P2 -> P2a [label="fork"];
  P2 -> P2b [label="fork"];
  P2a -> P3  [label="pipe_csv"];
  P3  -> CSV [label="write"];
  P2b -> P4  [label="pipe_freqtab"];
  P4  -> P5  [label="pipe_out"];
  P5  -> TXT [label="write"];
  TXT -> OUT  [style=dashed, label="if empty"];
}
\enddot

### Task1 process flow
\dot
digraph Task1 {
  rankdir=LR; nodesep=0.6; ranksep=0.7; splines=true;
  node [shape=box, style="rounded"];

  I   [label="input_txt", shape=oval, style=""];
  P1  [label="P1: w_p1\nopen → publish_word_on_pipe"];
  P2  [label="P2: w_p2\nread_words → HT → csv_line_send"];
  P3  [label="P3: w_p3\ncsv_line_recv → write"];
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
  P1  [label="P1: w_p1\nopen → publish_line_on_pipe → close"];
  P2  [label="P2: w_p2\nread_lines → HT → send_output_mp(n_words,word)"];
  P3  [label="P3: w_p3\nwrite_output_mp(output_txt)"];
  TXT [label="output_txt", shape=oval, style=""];
  OUT [label="stdout (se output_txt empty)", shape=oval, style=""];

  CSV -> P1;
  P1  -> P2 [label="pipe12"];
  P2  -> P3 [label="pipe23"];
  P3  -> TXT;
  TXT -> OUT [style=dashed];
}
\enddot



