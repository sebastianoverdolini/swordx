#ifndef TRIE_H
#define TRIE_H

#include "../list/list.h"
#include <stdbool.h>

typedef struct Trie Trie;

/**
 * @brief Alloca la memoria per un Trie
 * composto dal solo nodo radice.
 * 
 * @return Trie* Il puntatore al Trie creato
 * @return NULL Failure
 */
Trie *trie_new();

/**
 * @brief Libera la memoria riservata al Trie
 * 
 * @param trie Il Trie da distruggere
 */
void trie_destroy(Trie *trie);

/**
 * @brief Inserisce una parola all'interno del Trie
 * 
 * @param word La parola da inserire
 * @param trie Il trie a cui aggiungere la parola
 * @return 0 Success
 * @return -1 Failure
 */
int trie_insert(const char *word, Trie *trie);

int trie_insert_with_occ(const char *word, int occurrences, Trie *trie);

/**
 * @brief Rimuove una parola e tutte le sue 
 * occorrenze dal Trie. Se la parola non è
 * contenuta all'interno del Trie non viene
 * eseguita nessuna azione.
 * @param word La parola da rimuovere
 * @param trie Il trie da cui rimuovere la parola
 */
void trie_remove(const char *word, Trie *trie);

/**
 * @brief Verifica se il trie contiene la parola
 * specificata.
 * 
 * @param word La parola da verificare
 * @param trie Il trie in cui cercare
 * @return true La parola è contenuta nel Trie
 * @return false La parola non è contenuta nel Trie
 */
bool trie_contains(const char *word, const Trie *trie);

/**
 * @brief Restituisce il numero di occorrenze di una parola
 * all'interno del Trie
 * 
 * @param word La parola di cui ottenere le occorrenze
 * @param trie Il trie in cui cercare la parola
 * @return int Il numero di occorrenze della parola
 */
int trie_get_word_occurrences(const char *word, const Trie *trie);

/**
 * @brief Imposta le occorrenze di una parola all'interno del
 * Trie.
 * 
 * @param word La parola di cui settare le occorrenze
 * @param occurrences Il numero di occorrenze da settare
 * @param trie
 * @return 0 Success
 * @return -1 Failure 
 */
int trie_set_word_occurrences(const char *word, const int occurrences, Trie *trie);

/**
 * @brief Inserisce una lista di parole all'interno del Trie
 * 
 * @param wordlist La lista delle parole da inserire
 * @param trie Il trie in cui inserire le parole
 * @return 0 Success
 * @return -1 Failure
 */
int trie_insert_wordlist(const List *wordlist, Trie *trie);

/**
 * @brief Restituisce le parole presenti nel Trie sottoforma
 * di lista di stringhe.
 * Formato: word occurrences
 * 
 * @param trie 
 * @return List* Il puntatore alla lista creata
 * @return NULL Failure
 */
List *trie_get_wordlist(const Trie *trie);



#endif