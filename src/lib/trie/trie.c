#include "trie.h"
#include "../list/list.h"

#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <errno.h>

#define ALPHABET 36

typedef struct _TrieNode _TrieNode;

static _TrieNode *_node_new(const char prefix, _TrieNode *parent);
static void _node_destroy(_TrieNode *node);
static bool _word_format_is_valid(const char *word);
static void _node_insert(const char *word, _TrieNode *node);
static _TrieNode *_get_last_word_node(const char *word, const _TrieNode *node);
static int _get_children_array_pos(const char prefix);
static void _collect_words(const _TrieNode *node, List *wordlist, char *word);

typedef struct Trie {
    _TrieNode *root;
} Trie;

typedef struct _TrieNode {
    char prefix;
    int occurrences;
    bool is_leaf;
    bool is_word;
    struct _TrieNode *children[ALPHABET];
    struct _TrieNode *parent;
} _TrieNode;

Trie *trie_new(){
    Trie *trie = malloc(sizeof(Trie));
    if(!trie){
        return NULL;
    }
    trie->root = _node_new('\0', NULL);
    if(!trie->root){
        free(trie);
        return NULL;
    }
    return trie;
}

void trie_destroy(Trie *trie){
    if(trie){
        _node_destroy(trie->root);
        free(trie);
    }
}

int trie_insert(const char *word, Trie *trie){
    assert(trie);
    if(strcmp(word, "") == 0){
        errno = EINVAL;
        return -1;
    }
    if(!_word_format_is_valid(word)){
        errno = EINVAL;
        return -1;
    }
    _node_insert(word, trie->root);
    return 0;
}

bool trie_contains(const char *word, const Trie *trie){
    assert(trie);
    if(strcmp(word, "") == 0){
        return false;
    }
    if(!_word_format_is_valid(word)){
        return false;
    }
    if(_get_last_word_node(word, trie->root) == NULL){
        return false;
    } else {
        return true;
    }
}

int trie_get_word_occurrences(const char *word, const Trie *trie){
    assert(trie);
    if(strcmp(word, "") == 0){
        return 0;
    }
    if(!_word_format_is_valid(word)){
        return 0;
    }
    _TrieNode *node = _get_last_word_node(word, trie->root);
    return (node != NULL) ? node->occurrences : 0;
}

int trie_set_word_occurrences(const char *word, const int occurrences, Trie *trie){
    assert(trie);
    if(word == NULL){
        errno = EINVAL;
        return -1;
    }
    if(occurrences <= 0){
        errno = EINVAL;
        return -1;
    }
    if(strcmp(word, "") == 0){
        errno = EINVAL;
        return -1;
    }
    if(!_word_format_is_valid(word)){
        errno = EINVAL;
        return -1;
    }
    if(_get_last_word_node(word, trie->root) == NULL){
        int res = trie_insert(word, trie);
        if(res == -1){
            return -1;
        }
    }
    _TrieNode *node = _get_last_word_node(word, trie->root);
    node->occurrences = occurrences;
    return 0;
}

int trie_insert_wordlist(const List *wordlist, Trie *trie){
    assert(wordlist);
    assert(trie);
    ListIterator *iterator = list_iterator_new(wordlist);
    while(list_iterator_has_next(iterator)){
        list_iterator_advance(iterator);
        int res = trie_insert(list_iterator_get_element(iterator), trie);
        if(res == -1){
            return -1;
        }
    }
    return 0;
}

List *trie_get_wordlist(const Trie *trie){
    assert(trie);
    List *wordlist = list_new();
    if(!wordlist){
        return NULL;
    }
    _collect_words(trie->root, wordlist, '\0');
    if(!wordlist){
        return NULL;
    }
    return wordlist;
}

/* Private Methods */

static _TrieNode *_node_new(const char prefix, _TrieNode *parent){
    _TrieNode *node = malloc(sizeof(_TrieNode));
    if(!node){
        return NULL;
    }
    node->prefix = prefix;
    node->is_word = false;
    node->is_leaf = true;
    node->parent = parent;
    return node;
}

static void _node_destroy(_TrieNode *node){
    if(node){
        for(int i = 0; i <=ALPHABET; i++){
            _node_destroy(node->children[i]);
        }
        free(node);
    }
}

static bool _word_format_is_valid(const char *word){
    assert(word);
    for(int i = 0; i != strlen(word); i++){
        if(!isalnum(word[i])){
            return false;
        }
    }
    return true;
}

static void _node_insert(const char *word, _TrieNode *node){
    assert(node);
    if(strlen(word) == 0){
        node->occurrences++;
        node->is_word = true;
        return;
    }
    char next_prefix = tolower(word[0]);
    int next_child_index = _get_children_array_pos(next_prefix);
    assert(next_child_index != -1);
    if(node->children[next_child_index] == NULL){
        node->children[next_child_index] = _node_new(next_prefix, node);
        assert(node->children[next_child_index]);
        if(node->is_leaf){
            node->is_leaf = false;
        }
    }
    _node_insert(word+1, node->children[next_child_index]);
}

static _TrieNode *_get_last_word_node(const char *word, const _TrieNode *node){
    assert(node);
    if(strlen(word) == 0 && node->is_word == true){
        return node;
    }
    char next_prefix = tolower(word[0]);
    int next_child_index = _get_children_array_pos(next_prefix);
    if(next_child_index == -1){
        return NULL;
    }
    _TrieNode *next_child = node->children[next_child_index];
    if(next_child == NULL){
        return NULL;
    }
    return _get_last_word_node(word + 1, next_child);
}

static int _get_children_array_pos(const char prefix){
    int index = -1;
    if(isalpha(prefix)){
        index = prefix - 'a' + 10;
    }
    else if(isdigit(prefix)){
        index = prefix - '0';
    }
    if(index > 35 || index < 0){
        return -1;
    }
    return index;
}

static void _collect_words(const _TrieNode *node, List *wordlist, char *word){
    assert(node);
    assert(wordlist);
    if(node->is_word){
        list_append(word, wordlist);
    }
    if(node->is_leaf){
        return;
    }
    for(int i = 0; i < ALPHABET; i++){
        if(node->children[i] != NULL){
            int next_word_len;
            if(word != NULL){
                next_word_len = strlen(word) + 2;
            } else {
                next_word_len = 2;
            }
            char *next_word = (char *)malloc(next_word_len);
            assert(next_word);
            if(word == NULL){
                next_word[0] = node->children[i]->prefix;
                next_word[1] = '\0';
            } else {
                strcpy(next_word, word);
                int len = strlen(next_word);
                next_word[len] = node->children[i]->prefix;
                next_word[len + 1] = '\0';
            }
            _collect_words(node->children[i], wordlist, next_word);
        }
    }
}