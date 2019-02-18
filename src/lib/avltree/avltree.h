#ifndef AVLTREE_H
#define AVLTREE_H

#include <stdbool.h>
#include "../trie/trie.h"

typedef struct AVLTree AVLTree;

typedef struct AVLTreeIterator AVLTreeIterator;

AVLTree *avltree_new();

void avltree_destroy(AVLTree *tree);

int avltree_get_nodes_count(const AVLTree *tree);

int avltree_get_height(const AVLTree *tree);

int avltree_get_maximum_key(const AVLTree *tree);

int avltree_get_minimum_key(const AVLTree *tree);

Trie *avltree_get_element_by_key(const int key, const AVLTree *tree);

bool avltree_is_empty(const AVLTree *tree);

bool avltree_contains_key(const int key, const AVLTree *tree);

int avltree_insert(const int key, Trie *element, AVLTree *tree);

AVLTreeIterator *avltree_iterator_new(const AVLTree *tree);

void avltree_iterator_destroy(AVLTreeIterator *iterator);

bool avltree_iterator_has_next(const AVLTreeIterator *iterator);

void avltree_iterator_advance(AVLTreeIterator *iterator);

int avltree_iterator_get_key(const AVLTreeIterator *iterator);

#endif