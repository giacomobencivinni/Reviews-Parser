/* Header della Symbol Table per il progetto recensioni online */

#ifndef SYMBOL_TABLE_H
#define SYMBOL_TABLE_H

#include <stdio.h>

#define HASH_SIZE 101
#define MAX_NOME 100
#define MAX_CATEGORIA 50
#define MAX_ID 10

// Struttura articolo
typedef struct {
    char id_prodotto[MAX_ID];
    char nome_prodotto[MAX_NOME];
    char categoria[MAX_CATEGORIA];
    float prezzo;
    float somma_punteggi;
    float somma_prezzi_valutati;
    int num_recensioni;
} Articolo;

//Struttura utente
typedef struct {
    char nome_utente[MAX_NOME];
    char categoria_preferita[MAX_CATEGORIA];
} Utente;

//Struttura  recensione
typedef struct {
    char id_prodotto[MAX_ID];
    char nome_utente[MAX_NOME];
    int punteggio;
    int prezzo_valutato;
} Recensione;

//Entry per la hash table
typedef struct Entry {
    char *key;
    void *value;
    struct Entry *next;
} Entry;

//Hash table
typedef struct {
    Entry *table[HASH_SIZE];
} HashTable;

//Funzioni per la hash table
HashTable *create_table();
void insert(HashTable *ht, char *key, void *value);
void *search(HashTable *ht, char *key);
void free_table(HashTable *ht);

//Funzioni specifiche per il progetto
void inserisci_articolo(HashTable *articoli, Articolo *art);
void inserisci_utente(HashTable *utenti, Utente *user);
void elabora_recensione(HashTable *articoli, HashTable *utenti, Recensione *rec);
int conta_utenti_categoria(HashTable *utenti, char *categoria);
void stampa_risultati(HashTable *articoli, HashTable *utenti, FILE *output);

//Variabili globali
extern HashTable *tabella_articoli;
extern HashTable *tabella_utenti;

#endif