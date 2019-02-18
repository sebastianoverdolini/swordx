#ifndef LIST_H
#define LIST_H

typedef struct List List;
typedef struct ListIterator ListIterator;

/**
 * @brief Crea una nuova lista vuota
 * 
 * @return List* Puntatore alla lista creata
 * @return NULL Failure
 */
List *list_new();

/**
 * @brief Libera la memoria allocata per la lista
 * 
 * @param list La lista da distruggere
 */
void list_destroy(List *list);

/**
 * @brief Restituisce il numero dei nodi 
 * contenuti nella lista
 * 
 * @param list La lista di cui calcolare i nodi
 * @return int Il numero dei nodi
 */
int list_get_elements_count(const List *list);

/**
 * @brief Verifica se il valore specificato è
 * contenuto nella lista.
 * 
 * @param value Valore da ricercare
 * @param list Lista in cui cercare
 * @return true Il valore è contenuto nella lista
 * @return false Il valore non è contenuto nella lista
 */
bool list_contains(const char *value, const List *list);

/**
 * @brief Aggiunge un elemento alla lista
 * 
 * @param value L'elemento da aggiungere
 * @param list La lista a cui aggiungere l'elemento
 * @return 0 Success
 * @return -1 Failure 
 */
int list_append(const char *value, List *list);

/**
 * @brief Concatena i nodi della lista source
 * alla lista destination-
 * 
 * @param source 
 * @param destination 
 * @return 0 Success
 * @return -1 Failure 
 */
int list_addall(const List *source, List *destination);

/**
 * @brief Crea un iteratore associato alla lista specificata
 * 
 * @param list La lista su cui creare l'iteratore
 * @return ListIterator* Il puntatore all'iteratore creato
 * @return NULL Failure
 */
ListIterator *list_iterator_new(const List *list);

/**
 * @brief Libera la memoria allocata per l'iteratore specificato.
 * 
 * @param iterator L'iteratore di cui si vuole liberare la memoria.
 */
void list_iterator_destroy(ListIterator *iterator);

/**
 * @brief Controlla se il nodo associato all'iteratore possiede un successivo
 * 
 * @param iterator
 * @return true Il nodo possiede un successivo
 * @return false Il nodo non possiede un successivo
 */
bool list_iterator_has_next(const ListIterator *iterator);

/**
 * @brief Avanza il nodo associato all'iteratore di un passo
 * 
 * @param iterator 
 */
void list_iterator_advance(ListIterator *iterator);

/**
 * @brief Restituisce il valore contenuto nel nodo associato,
 * al momento della chiamata, all'iteratore
 * 
 * @param iterator 
 * @return char* 
 */
char *list_iterator_get_element(const ListIterator *iterator);

#endif