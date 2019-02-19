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

static void process_command(int argc, char *argv[], List *inputs);
static void collect_files(List *inputs);
static void collect_words(List *files, Trie *words, AVLTree *occurr_words);
static void save_output(char *output_path, Trie *words, AVLTree *occurr_words);

static void initialize_optargs();

int main(int argc, char *argv[]){
    initialize_optargs();
    List *inputs = list_new();
    if(!inputs) die(NULL);
    List *files = list_new();
    if(!files) die(NULL);
    Trie *words = trie_new();
    if(!words) die(NULL);
    AVLTree *occurr_words = avltree_new();
    if(!occurr_words) die(NULL);

    process_command(argc, argv, inputs);
    collect_files(inputs);
    collect_words(files, words, occurr_words);
    save_output(OptArgs.output_path, words, occurr_words);

    list_destroy(inputs);
    list_destroy(files);
    trie_destroy(words);
    avltree_destroy(occurr_words);
    free_optargs();
}

static void initialize_optargs(){
    recursive = false;
    follow = false;
    alpha = false;
    sortbyoccurrency = false;
    update = false;
    log = false;

    OptArgs.files_to_exclude = list_new();
    OptArgs.minimum_word_length = 0;
    OptArgs.words_to_ignore = trie_new();
}