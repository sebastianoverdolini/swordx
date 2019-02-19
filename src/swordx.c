#include "lib/list/list.h"
#include "lib/avltree/avltree.h"
#include "lib/trie/trie.h"

#include <stdio.h>

void test_list_base();

int main(){
    test_list_base();
}

void test_list_base(){
    List *list = list_new();
    list_append("Prova", list);
    list_append("Cazzo", list);
    list_append("Merda", list);

    List *second_list = list_new();
    list_append("Ciaone", second_list);
    list_append("Bona", second_list);

    list_addall(second_list, list);

    if(list_contains("Bona", list) && !list_contains("Fika", list)){
        printf("CONTAINS FUNZIONANTE");
    }

    printf("elements count (3): %d", list_get_elements_count(list));

    ListIterator *iterator = list_iterator_new(list);
    while(list_iterator_has_next(iterator)){
        list_iterator_advance(iterator);
        printf(list_iterator_get_element(iterator));
    }

    list_destroy(list);
}