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


