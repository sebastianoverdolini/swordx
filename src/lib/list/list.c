#include "list.h"

#include <stdbool.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include <errno.h>

typedef struct _ListNode _ListNode;
static bool _iterator_has_next(const ListIterator *iterator);
static void _iterator_advance(ListIterator *iterator);

typedef struct List {
    int elements_count;
    _ListNode *head;
    _ListNode *tail;
} List;

typedef struct _ListNode {
    char *value;
    struct _ListNode *next;
} _ListNode;

typedef struct ListIterator {
    const List *list;
    _ListNode *actual_node;
    void (*f_advance)(ListIterator *self);
} ListIterator;

List *list_new(){
    List *list = malloc(sizeof(List));
    if(!list){
        return NULL;
    }
    list->elements_count = 0;
    list->head = NULL;
    list->tail = NULL;
    return list;
}

void list_destroy(List *list){
    if(list == NULL){
        return;
    }
    _ListNode *current_node;
    while(list->head != NULL){
        current_node = list->head;
        list->head = list->head->next;
        free(current_node);
    }
    list->head = NULL;
    free(list);
}

int list_get_elements_count(const List *list){
    assert(list);
    return list->elements_count;
}

bool list_contains(const char *value, const List *list){
    assert(list);
    if(!list->head){
        return false;
    }
    ListIterator *iterator = list_iterator_new(list);
    assert(iterator);
    while(list_iterator_has_next(iterator)){
        list_iterator_advance(iterator);
        if(strcmp(list_iterator_get_element(iterator), value) == 0){
            return true;
        }
    }
    list_iterator_destroy(iterator);
    return false;
}

int list_append(const char *value, List *list){
    assert(list);
    if(!value){
        errno = EINVAL;
        return -1;
    }
    _ListNode *new_node = malloc(sizeof(_ListNode));
    if(!new_node){
        return -1;
    }
    new_node->value = malloc(strlen(value) + 1);
    if(!new_node->value){
        return -1;
    }
    strcpy(new_node->value, value);
    new_node->next = NULL;
    if(list->elements_count == 0){
        list->head = list->tail = new_node;
    } else {
        list->tail->next = new_node;
        list->tail = new_node;
    }
    list->elements_count++;
    return 0;
}

ListIterator *list_iterator_new(const List *list){
    assert(list);
    ListIterator *iterator = malloc(sizeof(ListIterator));
    if(!iterator){
        return NULL;
    }
    iterator->list = list;
    iterator->actual_node = NULL;
    iterator->f_advance = _iterator_advance;
    return iterator;
}

void list_iterator_destroy(ListIterator *iterator){
    free(iterator);
}

bool list_iterator_has_next(const ListIterator *iterator){
    assert(iterator);
    return _iterator_has_next(iterator);
}

void list_iterator_advance(ListIterator *iterator){
    assert(iterator);
    iterator->f_advance(iterator);
}

char *list_iterator_get_element(const ListIterator *iterator){
    assert(iterator);
    return iterator->actual_node->value;
}

static bool _iterator_has_next(const ListIterator *iterator){
    assert(iterator);
    if(iterator->actual_node == NULL){
        if(iterator->list->head == NULL){
            return false;
        } else {
            return true;
        }
    }
    else if(iterator->actual_node->next == NULL){
        return false;
    }
    return true;
}

static void _iterator_advance(ListIterator *iterator){
    assert(iterator);
    if(iterator->actual_node == NULL){
        assert(iterator->list->head);
        iterator->actual_node = iterator->list->head;
    } else {
        assert(iterator->actual_node->next);
        iterator->actual_node = iterator->actual_node->next;
    }
}