#ifndef AVLTREE_H
#define AVLTREE_H

#include <stdbool.h>
#include "../trie/trie.h"

typedef struct AVLTree AVLTree;

typedef struct AVLTreeIterator AVLTreeIterator;

/**
 * @brief Crea un nuovo AVLTree vuoto
 * 
 * @return AVLTree* Il puntatore all'AVLTree creato
 * @return NULL Failure
 */
AVLTree *avltree_new();

/**
 * @brief Libera la memoria allocata per l'AVLTree
 * specificato.
 * 
 * @param tree L'AVLTree da distruggere
 */
void avltree_destroy(AVLTree *tree);

/**
 * @brief Restituisce il numero dei nodi dell'AVLTree
 * 
 * @param tree 
 * @return int 
 */
int avltree_get_nodes_count(const AVLTree *tree);

/**
 * @brief Restituisce l'altezza dell'albero
 * 
 * @param tree 
 * @return int 
 */
int avltree_get_height(const AVLTree *tree);

/**
 * @brief Restituisce la chiave più grande presente
 * nell'albero
 * 
 * @param tree 
 * @return int 
 */
int avltree_get_maximum_key(const AVLTree *tree);

/**
 * @brief Restituisce la chiave più piccola presente
 * nell'albero
 * 
 * @param tree 
 * @return int 
 */
int avltree_get_minimum_key(const AVLTree *tree);

/**
 * @brief Restituisce l'elemento associato alla chiave
 * specificata nell'albero
 * 
 * @param key 
 * @param tree 
 * @return Trie* Il puntatore all'elemento trovato
 * @return NULL Non è stato trovato nessun elemento
 */
Trie *avltree_get_element_by_key(const int key, const AVLTree *tree);

/**
 * @brief Verifica se l'AVLTree è vuoto
 * 
 * @param tree 
 * @return true 
 * @return false 
 */
bool avltree_is_empty(const AVLTree *tree);

/**
 * @brief Verifica se l'AVLTree contiene la chiave specificata
 * 
 * @param key 
 * @param tree 
 * @return true 
 * @return false 
 */
bool avltree_contains_key(const int key, const AVLTree *tree);

/**
 * @brief Inserisce una chiave e l'elemento associato ad essa
 * all'interno dell'AVLTree
 * 
 * @param key 
 * @param element 
 * @param tree 
 * @return 0 Success
 * @return -1 Failure
 */
int avltree_insert(const int key, Trie *element, AVLTree *tree);

/**
 * @brief Crea un nuovo iteratore associato all'albero
 * 
 * @param tree 
 * @return AVLTreeIterator* Puntatore all'iteratore creato
 * @return NULL Failure
 */
AVLTreeIterator *avltree_iterator_new(const AVLTree *tree);

/**
 * @brief Distrugge l'iteratore specificato
 * 
 * @param iterator 
 */
void avltree_iterator_destroy(AVLTreeIterator *iterator);

/**
 * @brief Controlla se il nodo associato all'iteratore possiede un successivo
 * 
 * @param iterator
 * @return true Il nodo possiede un successivo
 * @return false Il nodo non possiede un successivo
 */
bool avltree_iterator_has_next(const AVLTreeIterator *iterator);

/**
 * @brief Avanza il nodo associato all'iteratore di un passo
 * 
 * @param iterator 
 */
void avltree_iterator_advance(AVLTreeIterator *iterator);

/**
 * @brief Restituisce la chiave contenuta nel nodo associato,
 * al momento della chiamata, all'iteratore
 * 
 * @param iterator 
 * @return char* 
 */
int avltree_iterator_get_key(const AVLTreeIterator *iterator);

/**
 * @brief Restituisce l'elemento contenuto nel nodo associato,
 * al momento della chiamata, all'iteratore
 * 
 * @param iterator 
 * @return char* 
 */
Trie *avltree_iterator_get_element(const AVLTreeIterator *iterator);

#endif