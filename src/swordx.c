#define _GNU_SOURCE

#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <ctype.h>
#include <assert.h>
#include <getopt.h>
#include <limits.h>

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
void exit_success();
static void die(char *message);
static void free_global();

char *get_absolute_path(const char *path);
int convert_to_int(const char *text);
void print_help();

int main(int argc, char *argv[]){
    initialize_global();
    List *inputs = list_new();
    if(!inputs) die(NULL);
    Trie *words = trie_new();
    if(!words) die(NULL);
    AVLTree *occurr_words = avltree_new();
    if(!occurr_words) die(NULL);

    process_command(argc, argv, inputs);
    //collect_files(inputs);
    //collect_words(files, words, occurr_words);
    //save_output(OptArgs.output_path, words, occurr_words);

    list_destroy(inputs);
    trie_destroy(words);
    avltree_destroy(occurr_words);
    free_global();
}

static void process_command(int argc, char *argv[], List *inputs){
    assert(inputs);
    if(argc <= 1){
        print_help();
        errno = EIO;
        die("No parameters has been specified");
    }

    const struct option LongOpts[] = {
        {"help", no_argument, NULL, 'h'},
        {"recursive", no_argument, NULL, 'r'},
        {"follow", no_argument, NULL, 'f'},
        {"exclude", required_argument, NULL, 'e'},
        {"alpha", no_argument, NULL, 'a'},
        {"min", required_argument, NULL, 'm'},
        {"ignore", required_argument, NULL, 'i'},
        {"sortbyoccurrency", no_argument, NULL, 's'},
        {"log", required_argument, NULL, 'l'},
        {"update", no_argument, NULL, 'u'},
        {"output", required_argument, NULL, 'o'},
        {NULL, no_argument, NULL, 0}
    };
    const char *short_opts = "hrfe:am:i:sl:uo:";
    
    int option_index = 0;
    int opt;

    while( (opt = getopt_long(argc, argv, short_opts, LongOpts, &option_index) ) != -1){
        switch(opt){
            case 'h': print_help(); exit_success(); 
                break;
            case 'r': recursive = true;
                break;
            case 'f': follow = true;
                break;
            case 'e': {
                char *abspath = get_absolute_path(optarg);
                if(!abspath){
                    die("Error with exclude file.");
                }
                list_append(abspath, OptArgs.files_to_exclude);
            }
                break;
            case 'a': alpha = true;
                break;
            case 'm': {
                int min = convert_to_int(optarg);
                if(min < 0){
                    errno = EIO;
                    die("Invalid --minimum argument");
                } else {
                    OptArgs.minimum_word_length = min;
                }
            } break;
            case 'i': {
                List *filewords = get_words_from_file(optarg);
                if(!filewords){
                    die("Ignore file not valid");
                }
                int res = trie_insert_wordlist(filewords, OptArgs.words_to_ignore);
                if(res == -1){
                    die("Ignore file not valid");
                }
            } break;
            case 's': sortbyoccurrency = true;
                break;
            case 'l': log = true; OptArgs.log_path = optarg;
                break;
            case 'u': update = true;
                break;
            case 'o': OptArgs.output_path = optarg;
                break;
            case '?': print_help(); die("Option not valid");
                break;
            default: print_help(); die("Option not valid");
                break;
        }
    }

    if(OptArgs.output_path == NULL){
        OptArgs.output_path = "swordx.out";
    }

    for(int i = optind; i<argc; i++){
        char *abspath = get_absolute_path(argv[i]);
        if(!abspath){
            die("Error with input");
        }
        list_append(abspath, inputs);
    }

    if(list_get_elements_count(inputs) == 0){
        print_help();
        errno = EIO;
        die("No input to be processed has been specified");
    }
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

void exit_success(){
    free_global();
    exit(EXIT_SUCCESS);
}

static void die(char *message){
    perror(message);
    free_global();
    exit(EXIT_FAILURE);
}

static void free_global(){
    list_destroy(OptArgs.files_to_exclude);
    trie_destroy(OptArgs.words_to_ignore);
    free(OptArgs.output_path);
    free(OptArgs.log_path);
    list_destroy(files);
}

char *get_absolute_path(const char *path){
    char actual_path [PATH_MAX + 1];
    char *abspath = realpath(path, actual_path);
    if(abspath == NULL){
        return NULL;
    }
    return abspath;
}

int convert_to_int(const char *text){
    int len = strlen(text);
    if(len == 0){
        return -1;
    }
    int result = 0;
    for(int i = 0; i<len; i++){
        if(!isdigit(text[i])){
            return -1;
        }
        result = result * 10 + (text[i] - '0');
    }
    return result;
}

void print_help(){
    printf("\tUsage: swordx [options] [inputs]\n\n");
    printf("  HELP:\n");
    printf("\t-h / --help : help\n");
    printf("  OUTPUTS:\n");
    printf("\t-o / --output : output filename\n");
    printf("\t-l / --log <file> : viene generato un file di log\n");
    printf("\t-s / --sortbyoccurrency : le parole nel file di output vengono inserite per numero di occorrenze\n");
    printf("\t--update <file> : viene fatto update\n");
    printf("  FOLDERS:\n");
    printf("\t-r / --recursive : all subdirectories are followed in the process\n");
    printf("\t-f / --follow : links are followed in the process\n");
    printf("\t-e / --exclude <file> the specified file is not considered in processing\n");
    printf("  WORDS:\n");
    printf("\t-a / --alpha : only words containing alphabetic characters are considered in the statistics\n");
    printf("\t-m / --min <num> : the minimum word length\n");
    printf("\t-i / --ignore <file> : the file is list of word (one for lines) who ignored in the stats\n");
}