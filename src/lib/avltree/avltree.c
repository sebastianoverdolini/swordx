#include "avltree.h"
#include <stdlib.h>
#include <assert.h>

typedef struct _AVLTreeNode _AVLTreeNode;
static bool _iterator_has_next(const AVLTreeIterator *iterator);
static void _iterator_advance(AVLTreeIterator *iterator);

static void _rebalance(_AVLTreeNode *critical_node);
static void _rotate_left(_AVLTreeNode *critical_node);
static void _rotate_right(_AVLTreeNode *critical_node);

static void _insert(const int key, Trie *element, _AVLTreeNode *root);
static bool _is_balanced(const _AVLTreeNode *node);
static int _get_balance_factor(const _AVLTreeNode *node);
static void _update_height(_AVLTreeNode *node);
static bool _is_leaf(const _AVLTreeNode *node);
static _AVLTreeNode *_get_node_from_key(const int key, _AVLTreeNode *root);
static _AVLTreeNode *_get_minimum(_AVLTreeNode *node);
static _AVLTreeNode *_get_maximum(_AVLTreeNode *node);
static _AVLTreeNode *_get_successor(_AVLTreeNode *node);
static void _destroy(_AVLTreeNode *root);
static _AVLTreeNode *_new(const int key);

typedef struct AVLTree{
    _AVLTreeNode *root;
    int elements_count;
} AVLTree;

typedef struct _AVLTreeNode{
    int key;
    Trie *element;
    int height;
    _AVLTreeNode *left;
    _AVLTreeNode *right;
    _AVLTreeNode *parent;
} _AVLTreeNode;


typedef struct AVLTreeIterator{
    const AVLTree *tree;
    _AVLTreeNode *actual_node;
    void (*f_advance)(AVLTreeIterator *self);
} AVLTreeIterator;

AVLTree *avltree_new(){
    AVLTree *tree = malloc(sizeof(AVLTree));
    if(!tree){
        return NULL;
    }
    tree->elements_count = 0;
    tree->root = NULL;
    return tree;
}

void avltree_destroy(AVLTree *tree){
    _destroy(tree->root);
    free(tree);
}

int avltree_get_nodes_count(const AVLTree *tree){
    assert(tree);
    return tree->elements_count;
}

int avltree_get_height(const AVLTree *tree){
    assert(tree);
    return tree->root->height;
}

int avltree_get_maximum_key(const AVLTree *tree){
    assert(tree);
    return _get_maximum(tree->root)->key;
}

int avltree_get_minimum_key(const AVLTree *tree){
    assert(tree);
    return _get_minimum(tree->root)->key;
}

Trie *avltree_get_element_by_key(const int key, const AVLTree *tree){
    assert(tree);
    _AVLTreeNode *node = _get_node_from_key(key, tree->root);
    return (node != NULL) ? node->element : NULL;
}

bool avltree_is_empty(const AVLTree *tree){
    assert(tree);
    return (tree->root == NULL);
}

bool avltree_contains_key(const int key, const AVLTree *tree){
    assert(tree);
    _AVLTreeNode *node = _get_node_from_key(key, tree->root);
    return (node != NULL) ? true : false;
}

int avltree_insert(const int key, Trie *element, AVLTree *tree){
    assert(tree);
    if(tree->root == NULL){
        tree->root = _new(key);
        if(!tree->root){
            return -1;
        }
        tree->root->element = malloc(sizeof(element));
        if(!tree->root->element){
            return -1;
        }
        tree->root->element = element;
        tree->elements_count++;
        tree->elements_count++;
    } else {
        _AVLTreeNode *node = _get_node_from_key(key, tree->root);
        if(node == NULL){
            _insert(key, element, tree->root);
            tree->elements_count++;
            while(tree->root->parent != NULL){
                tree->root = tree->root->parent;
            }
        } else {
            node->element = element;
        }
    }
    return 0;
}

AVLTreeIterator *avltree_iterator_new(const AVLTree *tree){
    assert(tree);
    AVLTreeIterator *iterator = malloc(sizeof(AVLTreeIterator));
    if(iterator == NULL){
        return NULL;
    }
    iterator->tree = tree;
    iterator->actual_node = NULL;
    iterator->f_advance = _iterator_advance;
    return iterator;
}

void avltree_iterator_destroy(AVLTreeIterator *iterator){
    free(iterator);
}

bool avltree_iterator_has_next(const AVLTreeIterator *iterator){
    assert(iterator);
    return _iterator_has_next(iterator);
}

void avltree_iterator_advance(AVLTreeIterator *iterator){
    assert(iterator);
    iterator->f_advance(iterator);
}

int avltree_iterator_get_key(const AVLTreeIterator *iterator){
    assert(iterator);
    return iterator->actual_node->key;
}

Trie *avltree_iterator_get_element(const AVLTreeIterator *iterator){
    assert(iterator);
    return iterator->actual_node->element;
}

static bool _iterator_has_next(const AVLTreeIterator *iterator){
    assert(iterator);
    if(iterator->tree->root == NULL){
        return false;
    }
    if(iterator->actual_node == NULL){
        return (_get_minimum(iterator->tree->root) != NULL);
    }
    return (_get_successor(iterator->actual_node) != NULL);
}

static void _iterator_advance(AVLTreeIterator *iterator){
    assert(iterator);
    assert(iterator->tree->root);
    assert(_get_maximum(iterator->tree->root) != iterator->actual_node);
    if(iterator->actual_node == NULL){
        iterator->actual_node = _get_minimum(iterator->tree->root);
    } else {
        iterator->actual_node = _get_successor(iterator->actual_node);
    }
}

static void _rebalance(_AVLTreeNode *critical_node){
    assert(critical_node);
    if(_get_balance_factor(critical_node) < 0){
        if(_get_balance_factor(critical_node->right) > 0){
            _rotate_right(critical_node->right);
        }
        _rotate_left(critical_node);
    } else if(_get_balance_factor(critical_node) > 0){
        if(_get_balance_factor(critical_node->left) < 0){
            _rotate_left(critical_node->left);
        }
        _rotate_right(critical_node);
    }
}
static void _rotate_left(_AVLTreeNode *critical_node){
    assert(critical_node);
    _AVLTreeNode *new_root = critical_node->right;
    critical_node->right = new_root->left;
    if(new_root->left != NULL){
        new_root->left->parent = critical_node;
    }
    if(critical_node->parent != NULL){
        if(critical_node->parent->right == critical_node){
            critical_node->parent->right = new_root;
        } else {
            critical_node->parent->left = new_root;
        }
    }
    new_root->left = critical_node;
    new_root->parent = critical_node->parent;
    critical_node->parent = new_root;
    _update_height(critical_node);
    _update_height(new_root);
}

static void _rotate_right(_AVLTreeNode *critical_node){
    assert(critical_node);
    _AVLTreeNode *new_root = critical_node->left;
    critical_node->left = new_root->right;
    if(new_root->right != NULL){
        new_root->right->parent = critical_node;
    }
    if(critical_node->parent != NULL){
        if(critical_node->parent->right == critical_node){
            critical_node->parent->right = new_root;
        } else {
            critical_node->parent->left = new_root;
        }
    }
    new_root->right = critical_node;
    new_root->parent = critical_node->parent;
    critical_node->parent = new_root;
    _update_height(critical_node);
    _update_height(new_root);
}

static void _insert(const int key, Trie *element, _AVLTreeNode *root){
    assert(root);
    if(key == root->key){
        return;
    }
    if(key < root->key){
        if(root->left == NULL){
            root->left = _new(key);
            root->left->element = element;
            root->left->parent = root;
        } else {
            _insert(key, element, root->left);
        }
    } else {
        if(root->right == NULL){
            root->right = _new(key);
            root->right->element = element;
            root->right->parent = root;
        } else {
            _insert(key, element, root->right);
        }
    }
    _update_height(root);
    if(!_is_balanced(root)){
        _rebalance(root);
    }
}

static bool _is_balanced(const _AVLTreeNode *node){
    assert(node);
    int balance_factor = _get_balance_factor(node);
    if(balance_factor < -1 || balance_factor > 1){
        return false;
    } else {
        if(node->left != NULL){
            _is_balanced(node->left);
        }
        if(node->right != NULL){
            _is_balanced(node->right);
        }
    }
    return true;
}

static int _get_balance_factor(const _AVLTreeNode *node){
    assert(node);
    if(_is_leaf(node)){
        return 0;
    }
    if (node->left != NULL && node->right == NULL){
        return node->left->height + 1;
    }
    if (node->left == NULL && node->right != NULL){
        return (node->right->height + 1) * -1;
    }
    else{
        return node->left->height - node->right->height;
    }
}

static void _update_height(_AVLTreeNode *node){
    assert(node);
    if(_is_leaf(node)){
        node->height = 0;
    } else {
        int left_height = (node->left != NULL) ? node->left->height : 0;
        int right_height = (node->right != NULL) ? node->right->height : 0;
        node->height = (left_height > right_height) ? (left_height + 1) : (right_height + 1);
    }
}

static bool _is_leaf(const _AVLTreeNode *node){
    assert(node);
    return (node->right == NULL) && (node->left == NULL);
}

static _AVLTreeNode *_get_node_from_key(int key, _AVLTreeNode *root){
    if(root == NULL){
        return NULL;
    }
    if(key == root->key){
        return root;
    } else if(key < root->key){
        if(root->left != NULL){
            return _get_node_from_key(key, root->left);
        } else {
            return NULL;
        }
    } else {
        if(root->right != NULL){
            return _get_node_from_key(key, root->right);
        } else {
            return NULL;
        }
    }
}

static _AVLTreeNode *_get_minimum(_AVLTreeNode *node){
    assert(node);
    if(node->left != NULL){
        return _get_minimum(node->left);
    }
    return node;
}

static _AVLTreeNode *_get_maximum(_AVLTreeNode *node){
    assert(node);
    if(node->right != NULL){
        return _get_maximum(node->right);
    }
    return node;
}

static _AVLTreeNode *_get_successor(_AVLTreeNode *node){
    assert(node);
    if(node->right != NULL){
        return _get_minimum(node->right);
    }
    _AVLTreeNode *parent = node->parent;
    _AVLTreeNode *n = node;
    while(parent != NULL && parent->right == n){
        n = parent;
        parent = n->parent;
    }
    return parent;
}

static void _destroy(_AVLTreeNode *root){
    if(!root){
        return;
    }
    _AVLTreeNode *node = _get_minimum(root);
    while(_get_successor(node) != NULL){
        _AVLTreeNode *to_destroy = node;
        node = _get_successor(node);
        free(to_destroy);
    }
}

static _AVLTreeNode *_new(const int key){
    _AVLTreeNode *new_node = malloc(sizeof(_AVLTreeNode));
    if(!new_node){
        return NULL;
    }
    new_node->key = key;
    return new_node;
}