# Reviews-Parser
Flex/Bison-based translator for analyzing an online review system

## Obiettivo del Progetto

Il progetto si propone di realizzare un traduttore basato sulla coppia FLEX/BISON per l'analisi di un sistema di recensioni online. L'obiettivo principale è trasformare un file di input strutturato contenente informazioni su prodotti, utenti e valutazioni in un report aggregato che presenti per ogni prodotto le medie delle valutazioni ricevute e il numero di utenti interessati alla categoria del prodotto.

Il sistema deve processare tre sezioni distinte:

- **Prima sezione**: catalogo prodotti con codici identificativi, nomi, categorie e prezzi
- **Seconda sezione**: database utenti con nomi e categorie preferite
- **Terza sezione**: recensioni degli utenti sui prodotti con valutazioni su qualità e rapporto qualità-prezzo

Deve infine fornire in output un file dove ogni riga riporta il nome del prodotto seguito dalle medie delle valutazioni (qualità e prezzo) e dal conteggio degli utenti in cui il prodotto recensito appartiene alla categoria preferita.

---

## Progettazione dello Scanner

Per gestire correttamente i cambi di contesto tra le tre sezioni del file ho scelto di implementare un sistema a stati multipli ed esclusivi con il comando `\x`. In particolare, questa scelta è stata dettata dalla presenza di pattern simili e generali in contesti diversi che nelle fasi iniziali di progettazione hanno generato vari errori. (Utente - Categoria)

Gli stati implementati sono:

- **INITIAL**: stato di partenza per la prima sezione (prodotti)
- **STATO_FRECCIA**: stato transitorio per il riconoscimento dei valori dopo la freccia "->"
- **SEZIONE_UTENTI**: secondo stato per il parsing degli utenti
- **DOPO_TRATTINO**: stato per il riconoscimento delle categorie utente dopo il trattino
- **SEZIONE_RECENSIONI**: terzo stato per il parsing delle recensioni

### Motivazioni per la scelta degli stati

L'utilizzo di stati multipli deriva dalle seguenti considerazioni:

- Gli identificativi prodotto (`[a-z]{4}[0-9]{2}`) devono essere riconosciuti in contesti diversi (sezione articoli e recensioni)
- Le sequenze di asterischi e dollari sono specifiche della sezione recensioni
- La gestione dei separatori di sezione ("%%%") richiede transizioni di stato precise

La transizione tra stati è stata progettata per rispettare rigorosamente la struttura del file:

- Il separatore "%%%" attiva la transizione verso la sezione successiva
- Lo stato STATO_FRECCIA gestisce i valori che seguono "->" permettendo di distinguere tra stringhe (nomi/categorie) e numeri (prezzi)
- Lo stato DOPO_TRATTINO isola il riconoscimento delle categorie utente, evitando conflitti con altri pattern alfanumerici

---

## Progettazione del Parser

La grammatica è stata progettata per riflettere la struttura tripartita del file di input 

Definiamo la grammatica context free $G(\Sigma, N, S, P)$, dove:

```
Σ = { SEPARATORE_SEZIONI, SEPARATORE_PRODOTTI, NOME_PRODOTTO, CATEGORIA, PREZZO, FRECCIA, TRATTINO, PAR_AP, PAR_CH, VIRGOLA, ID_PRODOTTO, ID_UTENTE, VALORE_CAMPO, CATEGORIA_UTENTE, PUNTEGGIO, PREZZO_ARTICOLO, INT_NUM, FLOAT_NUM
}

N = { programma, input, sezione_articoli, lista_articoli, articolo, nome_campo,categoria_campo, prezzo_campo, sezione_utenti, lista_utenti, utente,sezione_recensioni, lista_recensioni, recensione
}

S = programma

P = {programma → input,
    input → sezione_articoli SEPARATORE_SEZIONI sezione_utenti SEPARATORE_SEZIONI sezione_recensioni,
    sezione_articoli → lista_articoli | ε,
    lista_articoli → articolo | lista_articoli SEPARATORE_PRODOTTI articolo,
    articolo → ID_PRODOTTO nome_campo categoria_campo prezzo_campo,
    nome_campo → NOME_PRODOTTO FRECCIA VALORE_CAMPO,
    categoria_campo → CATEGORIA FRECCIA VALORE_CAMPO,
    prezzo_campo → PREZZO FRECCIA FLOAT_NUM | PREZZO FRECCIA INT_NUM,
    sezione_utenti → lista_utenti | ε,
    lista_utenti → utente | lista_utenti utente,
    utente → ID_UTENTE TRATTINO CATEGORIA_UTENTE,
    sezione_recensioni → lista_recensioni | ε,
    lista_recensioni → recensione | lista_recensioni recensione,
    recensione → PAR_AP ID_PRODOTTO VIRGOLA ID_UTENTE PAR_CH PUNTEGGIO PREZZO_ARTICOLO
}
```

### Gestione delle Sezioni

Ogni sezione ha una grammatica specifica:

- **Sezione prodotti**: sequenza di blocchi prodotto separati da "&&&", dove ogni blocco contiene ID, nome, categoria e prezzo
- **Sezione utenti**: lista di definizioni utente nel formato "nome - categoria"
- **Sezione recensioni**: lista di valutazioni nel formato "(prodotto,utente) stelle dollari"

La scelta di utilizzare produzioni ricorsive a sinistra per le liste ottimizza l'utilizzo dello stack del parser e garantisce una costruzione incrementale delle strutture dati.

### Gestione dei tipi di token

È stata definita una `%union` in quanto i token possono essere di tipi diversi:

- stringhe
- valori interi
- valori float

Infatti bison genera la variabile globale yyval di tipo YYSTYPE, quando il lexer trova un token, comunica al parser sia il tipo d token che il suo valore con la scrittura: `yylval.int_val`, `yylval.float_val`, o `yylval.str`

```c
%union {
    int int_val;
    float float_val;
    char *str;
}

%token SEPARATORE_SEZIONI SEPARATORE_PRODOTTI NOME_PRODOTTO CATEGORIA PREZZO FRECCIA TRATTINO PAR_AP PAR_CH VIRGOLA
%token <str> ID_PRODOTTO ID_UTENTE VALORE_CAMPO CATEGORIA_UTENTE
%token <int_val> PUNTEGGIO PREZZO_ARTICOLO INT_NUM
%token <float_val> FLOAT_NUM
```

---

## Symbol Table

La symbol table è stata implementata come hash table per garantire accesso efficiente ai dati con complessità temporale media O(1). Questa scelta è motivata dalla natura del problema che richiede frequenti ricerche di prodotti e utenti durante l'elaborazione delle recensioni.

La symbol table utilizza una struttura basata su:

- **HashTable**: struttura principale contenente un array di puntatori alle entry
- **Entry**: nodo della lista collegata per la gestione delle collisioni
- **Strutture dati specifiche**: Articolo, Utente, Recensione per tipizzare le informazioni

## Funzione Hash Ottimizzata

```c
unsigned int hash(char *key) {
    unsigned long hash = 5381;
    int c;
    while ((c=*key++)) {
        hash = ((hash<<5) + hash) + c;
    }
    return (unsigned int)hash % HASH_SIZE;
}
```

La scelta di djb2 rispetto a una funzione hash polinomiale è stata motivata da diversi fattori:

- **Velocità computazionale**: djb2 utilizza operazioni bit-shift (`<<5`) e addizioni, significativamente più veloci delle classiche moltiplicazioni.
- **Qualità della distribuzione**: nonostante la semplicità, djb2 produce una distribuzione uniforme degli hash per stringhe tipiche, riducendo efficacemente le collisioni.
- **Stabilità numerica**:  djb2 mantiene caratteristiche di distribuzione stabili indipendentemente dalla lunghezza della chiave.

La scelta di HASH_SIZE=101  i numeri primi riduce significativamente le collisioni nella funzione modulo, migliorando la distribuzione delle chiavi nell'array.

Il valore iniziale 5381 non è un numero primo, che tipicamente viene utilizzato per distribuire in modo più uniforme le chiavi all’interno della tabella hash, è stato scelto empiricamente come seed della funzione hash.

## Gestione delle Collisioni: Concatenamento

```c
typedef struct Entry {
    char *key;
    void *value;
    struct Entry *next;
} Entry;
```

Il sistema implementa il concatenamento per gestire le collisioni , per cui ogni posizione dell'array (tabella hash) contiene una lista collegata di elementi che posseggono lo stesso valore hash.

### Approccio Generico con void*

La hash table implementa un approccio generico utilizzando puntatori `void*` per i valori:

```c
void insert(HashTable *ht, char *key, void *value);
void *search(HashTable *ht, char *key);
```

Questa scelta permette di utilizzare la stessa struttura dati per gestire sia articoli che utenti, evitando duplicazione di codice e mantenendo un'interfaccia uniforme.

---

## Integrazione con il sistema

La symbol table interagisce con il parser attraverso funzioni specifiche:

- `inserisci_articolo()`: inserisce un nuovo prodotto durante il parsing della prima sezione
- `inserisci_utente()`: registra un utente durante il parsing della seconda sezione
- `elabora_recensione()`: aggiorna le statistiche degli articoli durante il parsing delle recensioni

La separazione in due hash table distinte (articoli e utenti) ottimizza le ricerche e previene conflitti tra namespace diversi.

---

## Gestione degli Errori

Il sistema implementa una gestione degli errori multi-livello:

**Errori lessicali**: caratteri non riconosciuti sono segnalati con indicazione della riga, permettendo localizzazione precisa del problema.

**Errori sintattici**: BISON genera automaticamente messaggi per errori grammaticali, integrati con informazioni contestuali personalizzate.

**Errori semantici**: validazione dei dati (range delle valutazioni, esistenza di prodotti/utenti) con messaggi specifici per ogni tipo di inconsistenza.
