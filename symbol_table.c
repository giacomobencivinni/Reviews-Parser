#include "symbol_table.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

HashTable *tabella_articoli = NULL;
HashTable *tabella_utenti = NULL;

unsigned int hash(char *key) {
    unsigned long hash = 5381;
    int c; 
    while ((c=*key++)) {
        hash = ((hash<<5) + hash)+ c;
    }
    return (unsigned int)hash % HASH_SIZE;
}

HashTable *create_table() {
    HashTable *ht = malloc(sizeof(HashTable));
    if (!ht){
        fprintf(stderr, "ERRORE: Impossibile allocare memoria per la hash table\n");
        return NULL;
    }
    for (int i = 0; i < HASH_SIZE; i++) {
        ht->table[i] = NULL;
    }
    printf("Hash table creata con successo\n");
    return ht;
}

void insert(HashTable *ht, char *key, void *value) {
    if (!ht){
        fprintf(stderr, "ERRORE: Hash table NULL in insert\n");
        return;
    }
    if(!key){
        fprintf(stderr, "ERRORE: Chiave NULL in insert\n");
        return;
    }
    if(!value){
         fprintf(stderr, "ERRORE: Valore NULL in insert\n");
        return;
    }
    
    unsigned int index = hash(key);
    Entry *new_entry = malloc(sizeof(Entry));
    if (!new_entry){
        fprintf(stderr, "ERRORE: Impossibile allocare memoria per nuovo entry\n");
        return;
    }
    
    new_entry->key = strdup(key);
    if (!new_entry->key) {
        fprintf(stderr, "ERRORE: Impossibile duplicare la chiave\n");
        free(new_entry);
        return;
    }
    new_entry->value = value;
    new_entry->next = ht->table[index];
    ht->table[index] = new_entry;
    printf("Elemento inserito con chiave: %s\n", key);
}

void *search(HashTable *ht, char *key) {
    if (!ht){
        fprintf(stderr, "ERRORE: Hash table NULL in search\n");
        return NULL;
    }
    if(!key){
        fprintf(stderr, "ERRORE: Chiave NULL in search\n");
        return NULL;
    }
    
    unsigned int index = hash(key);
    Entry *entry = ht->table[index];
    
    while (entry) {
        if (strcmp(entry->key, key) == 0) {
            return entry->value;
        }
        entry = entry->next;
    }
    printf("Elemento con chiave '%s' non trovato\n", key);
    return NULL;
}

// Libera la memoria della hash table
void free_table(HashTable *ht) {
    if (!ht){
        fprintf(stderr, "ATTENZIONE: Tentativo di liberare hash table NULL\n");
        return;
    }
    
    int entries_liberate = 0;
    for (int i = 0; i < HASH_SIZE; i++) {
        Entry *entry = ht->table[i];
        while (entry) {
            Entry *temp = entry;
            entry = entry->next;
            free(temp->key);
            free(temp->value);
            free(temp);
            entries_liberate++;
        }
    }
    free(ht);
    printf("Hash table liberata: %d entries eliminate\n", entries_liberate);
}

// Inserisce un articolo nella tabella
void inserisci_articolo(HashTable *articoli, Articolo *art) {
    if (!articoli){
        fprintf(stderr, "ERRORE: Tabella articoli NULL in inserisci_articolo\n");
        return;
    }
    if(!art){
        fprintf(stderr, "ERRORE: Articolo NULL in inserisci_articolo\n");
        return;
    }

    if (search(articoli, art->id_prodotto)) {
        fprintf(stderr, "ATTENZIONE: Articolo con ID '%s' già esistente\n", art->id_prodotto);
        return;
    }
    
    Articolo *nuovo_art = malloc(sizeof(Articolo));
    if (!nuovo_art){
        fprintf(stderr, "ERRORE: Impossibile allocare memoria per nuovo articolo '%s'\n", art->id_prodotto);
        return;
    }
    
    *nuovo_art = *art;
    nuovo_art->somma_punteggi = 0.0;
    nuovo_art->somma_prezzi_valutati = 0.0;
    nuovo_art->num_recensioni = 0;
    
    insert(articoli, art->id_prodotto, nuovo_art);
    printf("Articolo inserito: %s (%s)\n", nuovo_art->nome_prodotto, nuovo_art->id_prodotto);
}

// Inserisce un utente nella tabella
void inserisci_utente(HashTable *utenti, Utente *user) {
    if (!utenti){
        fprintf(stderr, "ERRORE: Tabella utenti NULL in inserisci_utente\n");
        return;
    }
    if(!user){
        fprintf(stderr, "ERRORE: Utente NULL in inserisci_utente\n");
        return;
    }

    if (search(utenti, user->nome_utente)) {
        fprintf(stderr, "ATTENZIONE: Utente '%s' già esistente, sovrascrittura\n", user->nome_utente);
    }
    
    Utente *nuovo_utente = malloc(sizeof(Utente));
    if (!nuovo_utente){
        fprintf(stderr, "ERRORE: Impossibile allocare memoria per nuovo utente '%s'\n", user->nome_utente);
        return;
    }
    
    *nuovo_utente = *user;
    insert(utenti, user->nome_utente, nuovo_utente);
    printf("Utente inserito: %s (categoria: %s)\n", 
           nuovo_utente->nome_utente, nuovo_utente->categoria_preferita);
}

// Elabora una recensione
void elabora_recensione(HashTable *articoli, HashTable *utenti, Recensione *rec) {
    if (!articoli){
        fprintf(stderr, "ERRORE: Tabella articoli NULL in elabora_recensione\n");
        return;
    }
    if(!utenti){
        fprintf(stderr, "ERRORE: Tabella utenti NULL in elabora_recensione\n");
        return;
    }
    if(!rec){
        fprintf(stderr, "ERRORE: Recensione NULL in elabora_recensione\n");
        return;
    }
    
    Articolo *art = (Articolo *)search(articoli, rec->id_prodotto);
    if (!art){
        fprintf(stderr, "ERRORE: Articolo '%s' non trovato per la recensione\n", rec->id_prodotto);
        return;
    }

    Utente *user = (Utente *)search(utenti, rec->nome_utente);
    if (!user) {
        fprintf(stderr, "ERRORE: Utente '%s' non trovato per la recensione\n", 
                rec->nome_utente);
        return;
    }

    if (rec->punteggio < 0 || rec->punteggio > 5) {
        fprintf(stderr, "ATTENZIONE: Punteggio %d fuori range [0-5] per articolo '%s'\n", 
                rec->punteggio, rec->id_prodotto);
    }
    
    if (rec->prezzo_valutato < 0 || rec->prezzo_valutato > 5) {
        fprintf(stderr, "ATTENZIONE: Prezzo valutato %d fuori range [0-5] per articolo '%s'\n", 
                rec->prezzo_valutato, rec->id_prodotto);
    }
    
    art->somma_punteggi += rec->punteggio;
    art->somma_prezzi_valutati += rec->prezzo_valutato;
    art->num_recensioni++;

    printf("Recensione elaborata: %s -> %s (punteggio: %d, prezzo: %d)\n",rec->id_prodotto, rec->nome_utente, rec->punteggio, rec->prezzo_valutato);
}

// Conta gli utenti che prediligono una categoria specifica
int conta_utenti_categoria(HashTable *utenti, char *categoria) {
    if (!utenti) {
        fprintf(stderr, "ERRORE: Tabella utenti NULL in conta_utenti_categoria\n");
        return 0;
    }
    if (!categoria) {
        fprintf(stderr, "ERRORE: Categoria NULL in conta_utenti_categoria\n");
        return 0;
    }
    
    int count = 0;
    for (int i = 0; i < HASH_SIZE; i++) {
        Entry *entry = utenti->table[i];
        while (entry) {
            Utente *user = (Utente *)entry->value;
            if (user && strcmp(user->categoria_preferita, categoria) == 0) {
                count++;
            }
            entry = entry->next;
        }
    }
    printf("Utenti nella categoria '%s': %d\n", categoria, count);
    return count;
}

// Stampa i risultati finali
void stampa_risultati(HashTable *articoli, HashTable *utenti, FILE *output) {
    if (!articoli) {
        fprintf(stderr, "ERRORE: Tabella articoli NULL in stampa_risultati\n");
        return;
    }
    if (!utenti) {
        fprintf(stderr, "ERRORE: Tabella utenti NULL in stampa_risultati\n");
        return;
    }
    if (!output) {
        fprintf(stderr, "ERRORE: File output NULL in stampa_risultati\n");
        return;
    }
    
    for (int i = 0; i < HASH_SIZE; i++) {
        Entry *entry = articoli->table[i];
        while (entry) {
            Articolo *art = (Articolo *)entry->value;
            if (art && art->num_recensioni > 0) {
                float media_punteggi = art->somma_punteggi / art->num_recensioni;
                float media_prezzi = art->somma_prezzi_valutati / art->num_recensioni;
                int utenti_categoria = conta_utenti_categoria(utenti, art->categoria);
                
                fprintf(output, "%s -> %.1f* %.1f$ %d\n", 
                       art->nome_prodotto, 
                       media_punteggi, 
                       media_prezzi, 
                       utenti_categoria);
            }
            entry = entry->next;
        }
    }
}