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

static void initialize_global();
static void die(char *message);
static void free_global();

int main(int argc, char *argv[]){
    initialize_global();
    List *inputs = list_new();
    if(!inputs) die(NULL);
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
    free_global();
}

static void initialize_optargs(){
    recursive = false;
    follow = false;
    alpha = false;
    sortbyoccurrency = false;
    update = false;
    log = false;

    OptArgs.files_to_exclude = list_new();
    if(!OptArgs.files_to_exclude) die(NULL);
    OptArgs.minimum_word_length = 0;
    OptArgs.words_to_ignore = trie_new();
    if(!OptArgs.words_to_ignore) die(NULL);
    List *files = list_new();
    if(!files) die(NULL);
}

static void die(char *message){
    if(!message){
        perror();
    } else {
        perror(message);
    }
    free_optargs();
    exit(EXIT_FAILURE);
}

static void free_global(){
    list_destroy(OptArgs.files_to_exclude);
    trie_destroy(OptArgs.words_to_ignore);
    free(OptArgs.output_path);
    free(OptArgs.log_path);
    list_destroy(files);
}