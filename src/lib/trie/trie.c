#include "trie.h"
#include "../list/list.h"

#include <ctype.h>
#include <stdio.h>
#include <stdbool.h>
#include <assert.h>
#include <stddef.h>
#include <string.h>
#include <errno.h>
#include <stdlib.h>

#define ALPHABET 36

typedef struct _TrieNode _TrieNode;

static _TrieNode *_node_new(const char prefix, _TrieNode *parent);
static void _node_destroy(_TrieNode *node);
static bool _word_format_is_valid(const char *word);
static void _node_insert(const char *word, int occurrences, _TrieNode *node);
static _TrieNode *_get_last_word_node(const char *word, _TrieNode *node);
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
    if(strlen(word) == 0 || !_word_format_is_valid(word)){
        errno = EINVAL;
        return -1;
    }
    _node_insert(word, 1, trie->root);
    return 0;
}

int trie_insert_with_occ(const char *word, int occurrences, Trie *trie){
    assert(trie);
    if(strlen(word) == 0 || !_word_format_is_valid(word)){
        errno = EINVAL;
        return -1;
    }
    if(occurrences < 1){
        errno = EINVAL;
        return -1;
    }
    _node_insert(word, occurrences, trie->root);
    return 0;
}

void trie_remove(const char *word, Trie *trie){
    assert(trie);
    _TrieNode *node = _get_last_word_node(word, trie->root);
    if(node){
        node->occurrences = 0;
        node->is_word = false;
    }
}

bool trie_contains(const char *word, const Trie *trie){
    assert(trie);
    if(strlen(word) == 0 || !_word_format_is_valid(word)){
        return false;
    }
    return (_get_last_word_node(word, trie->root) != NULL);
}

int trie_get_word_occurrences(const char *word, const Trie *trie){
    assert(trie);
    if(strlen(word) == 0 || !_word_format_is_valid(word)){
        return 0;
    }
    _TrieNode *node = _get_last_word_node(word, trie->root);
    return (node != NULL) ? node->occurrences : 0;
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
    for(int i = 0; i<ALPHABET; i++){
        node->children[i] = NULL;
    }
    node->occurrences = 0;
    node->is_word = false;
    node->is_leaf = true;
    node->parent = parent;
    return node;
}

static void _node_destroy(_TrieNode *node){
    if(node){
        for(int i = 0; i < ALPHABET; i++){
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

static void _node_insert(const char *word, int occurrences, _TrieNode *node){
    assert(node);
    if(strlen(word) == 0){
        node->occurrences += occurrences;
        node->is_word = true;
    } else {
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
        _node_insert(word+1, occurrences, node->children[next_child_index]);
    }   
}

static _TrieNode *_get_last_word_node(const char *word, _TrieNode *node){
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
        int len = strlen(word) + 1  + sizeof(int) + 1;
        char *word_info = malloc(len);
        assert(word_info);
        snprintf(word_info, len, "%s %d", word, node->occurrences);
        list_append(word_info, wordlist);
    }
    if (!node->is_leaf){
        for (int i = 0; i < ALPHABET; i++){
            if (node->children[i] != NULL){
                int next_word_len = (word != NULL) ? (strlen(word) + 2) : 2;
                char *next_word = malloc(next_word_len);
                assert(next_word);
                if (word != NULL)
                    strcpy(next_word, word);
                next_word[next_word_len - 2] = node->children[i]->prefix;
                next_word[next_word_len - 1] = '\0';
                _collect_words(node->children[i], wordlist, next_word);
            }
        }
    }
}