#define _GNU_SOURCE

#include <stdbool.h>
#include <stdlib.h>

#include "lib/list/list.h"
#include "lib/trie/trie.h"
#include "lib/avltree/avltree.h"

static bool recursive;
static bool follow;
static bool alpha;
static bool sortbyoccurrency;
static bool update;
static bool log;

static struct OptArgs {
    List *files_to_exclude;
    Trie *words_to_ignore;
    unsigned int minimum_word_length;
    char *output_path;
    char *log_path;
} OptArgs;

static List *files;


int main(int argc, char *argv[]){
    initialize_optargs();
    List *inputs = list_new();
    if(!inputs) die(NULL);
    List *files = list_new();
    if(!files) die(NULL);
    Trie *words = trie_new();
    if(!words) die(NULL);
    AVLTree *words_from_occ = avltree_new();
    if(!words_from_occ) die(NULL);

    process_command(argc, argv, inputs);
    collect_files(inputs);
    collect_words(files, words, words_from_occ);
    save(files, words, words_from_occ);

    list_destroy(inputs);
    trie_destroy(words);
    avltree_destroy(words_from_occ);

    free_optargs();
}