%{
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "symbol_table.h"

extern int yylex();
extern int yyparse();
extern FILE *yyin;
extern FILE *output;
void yyerror(const char *s);

// Variabili temporanee per la costruzione delle strutture
Articolo articolo_temp;
Utente utente_temp;
Recensione recensione_temp;
%}

%union {
    int int_val;
    float float_val;
    char *str;
}

%token SEPARATORE_SEZIONI SEPARATORE_PRODOTTI NOME_PRODOTTO CATEGORIA PREZZO FRECCIA TRATTINO PAR_AP PAR_CH VIRGOLA
%token <str> ID_PRODOTTO ID_UTENTE VALORE_CAMPO CATEGORIA_UTENTE
%token <int_val> PUNTEGGIO PREZZO_ARTICOLO INT_NUM
%token <float_val> FLOAT_NUM

%start programma
%error-verbose
%%

programma: input;

input: sezione_articoli SEPARATORE_SEZIONI sezione_utenti SEPARATORE_SEZIONI sezione_recensioni;

sezione_articoli: lista_articoli | /*vuota*/;

lista_articoli: articolo | lista_articoli SEPARATORE_PRODOTTI articolo;

articolo: ID_PRODOTTO nome_campo categoria_campo prezzo_campo
        {
            strcpy(articolo_temp.id_prodotto, $1);
            inserisci_articolo(tabella_articoli, &articolo_temp);
            free($1);
        }
        ;

nome_campo: NOME_PRODOTTO FRECCIA VALORE_CAMPO { 
    strcpy(articolo_temp.nome_prodotto, $3);
    free($3);
    };

categoria_campo: CATEGORIA FRECCIA VALORE_CAMPO
{
    strcpy(articolo_temp.categoria, $3);
    free($3);
}
;

prezzo_campo: PREZZO FRECCIA FLOAT_NUM 
{
    articolo_temp.prezzo = $3;
}
|
PREZZO FRECCIA INT_NUM
{
    articolo_temp.prezzo = (float)$3;  // Conversione esplicita da int a float
    printf("Prezzo (int->float) impostato: %d -> %.2f\n", $3, (float)$3);
}
;

sezione_utenti: lista_utenti | /*vuota*/;

lista_utenti: utente | lista_utenti utente;

utente: ID_UTENTE TRATTINO CATEGORIA_UTENTE
      {
          strcpy(utente_temp.nome_utente, $1);
          strcpy(utente_temp.categoria_preferita, $3);
          inserisci_utente(tabella_utenti, &utente_temp);
          free($1);
          free($3);
      }
      ;

sezione_recensioni: lista_recensioni | /*vuota*/
                  ;

lista_recensioni: recensione
                | lista_recensioni recensione
                ;

recensione: PAR_AP ID_PRODOTTO VIRGOLA ID_UTENTE PAR_CH PUNTEGGIO PREZZO_ARTICOLO
          {
              strcpy(recensione_temp.id_prodotto, $2);
              strcpy(recensione_temp.nome_utente, $4);
              recensione_temp.punteggio = $6;
              recensione_temp.prezzo_valutato = $7;
              
              elabora_recensione(tabella_articoli, tabella_utenti, &recensione_temp);
              
              free($2);
              free($4);
          }
          ;

%%

void yyerror(const char *s) {
    fprintf(stderr, "Errore di parsing: %s\n", s);
}

int main(int argc, char **argv) {
    // Inizializza le tabelle hash
    tabella_articoli = create_table();
    tabella_utenti = create_table();
    
    if (!tabella_articoli || !tabella_utenti) {
        fprintf(stderr, "Errore nell'inizializzazione delle tabelle hash\n");
        return 1;
    }
    
    // Legge il file di input
    if (argc > 1) {
        yyin = fopen(argv[1], "r");
        if (!yyin) {
            fprintf(stderr, "Errore nell'apertura del file %s\n", argv[1]);
            free_table(tabella_articoli);
            free_table(tabella_utenti);
            return 1;
        }
    }
    
    // Avvia il parsing
    int result = yyparse();

    // Apre (o crea) il file di output
    FILE *output = fopen("output.txt", "w");
    if (!output) {
        perror("Errore apertura file output.txt");
        exit(EXIT_FAILURE);
    }

    // Stampa i risultati nel file
    stampa_risultati(tabella_articoli, tabella_utenti, output);

    fclose(output);
    
    // Chiude il file se era stato aperto
    if (argc > 1 && yyin) {
        fclose(yyin);
    }
    
    // Libera la memoria
    free_table(tabella_articoli);
    free_table(tabella_utenti);
    
    return result;
}