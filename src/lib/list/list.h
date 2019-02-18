#ifndef LIST_H
#define LIST_H

typedef struct List List;
typedef struct ListIterator ListIterator;

/**
 * @brief 
 * 
 * @return List*
 * @return NULL Failure
 */
List *list_new();

/**
 * @brief 
 * 
 * @param list 
 */
void list_destroy(List *list);

int list_get_elements_count(const List *list);

bool list_contains(const char *value, const List *list);

/**
 * @brief 
 * 
 * @param value 
 * @param list 
 * @return 0 Success
 * @return -1 Failure 
 */
int list_append(const char *value, List *list);

/**
 * @brief 
 * 
 * @param source 
 * @param destination 
 * @return 0 Success
 * @return -1 Failure 
 */
int list_addall(const List *source, List *destination);

/**
 * @brief 
 * 
 * @param list 
 * @return ListIterator* 
 * @return NULL Failure
 */
ListIterator *list_iterator_new(const List *list);

void list_iterator_destroy(ListIterator *iterator);

bool list_iterator_has_next(const ListIterator *iterator);

void list_iterator_advance(ListIterator *iterator);

char *list_iterator_get_element(const ListIterator *iterator);

#endif