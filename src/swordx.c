#define _GNU_SOURCE

#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <ctype.h>
#include <ftw.h>
#include <string.h>
#include <assert.h>
#include <getopt.h>
#include <limits.h>

#include "lib/list/list.h"
#include "lib/trie/trie.h"
#include "lib/avltree/avltree.h"

#define DEFAULT_OUTPUT_NAME "swordx.out"

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

void process_command(int argc, char *argv[], List *inputs);
void collect_files(List *inputs);
void collect_words(List *files, Trie *words, AVLTree *occurr_words);
void save_output(char *output_path, Trie *words, AVLTree *occurr_words);

void initialize_global();
void exit_success();
void die(char *message);
void free_global();

int manage_entry(const char *fpath, const struct stat *sb, int typeflag, struct FTW *ftbuf);
List *get_words_from_file(const char *path);
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
    collect_files(inputs);
    
    list_destroy(inputs);
    trie_destroy(words);
    avltree_destroy(occurr_words);
    free_global();
}

void process_command(int argc, char *argv[], List *inputs){
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
                    die("Invalid --exclude argument");
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
                    die("Invalid --ignore argument");
                }
                int res = trie_insert_wordlist(filewords, OptArgs.words_to_ignore);
                if(res == -1){
                    die("Invalid --ignore argument");
                }
            } break;
            case 's': sortbyoccurrency = true;
                break;
            case 'l': {
                 log = true;
                OptArgs.log_path = malloc(strlen(optarg) +1);
                if(!OptArgs.log_path){
                    die("Error with --log argument");
                }
                strcpy(OptArgs.log_path, optarg);
            }
                break;
            case 'u': update = true;
                break;
            case 'o': {
                OptArgs.output_path = malloc(strlen(optarg) +1);
                if(!OptArgs.output_path){
                    die("Error with --output argument");
                }
                strcpy(OptArgs.output_path, optarg);
            }
                break;
            case '?': print_help(); errno = EIO; die("Option not valid");
                break;
            default: print_help(); errno = EIO; die("Option not valid");
                break;
        }
    }

    if(OptArgs.output_path == NULL){
        OptArgs.output_path = malloc(strlen(DEFAULT_OUTPUT_NAME) +1);
        strcpy(OptArgs.output_path, DEFAULT_OUTPUT_NAME);
    }

    for(int i = optind; i<argc; i++){
        char *abspath = get_absolute_path(argv[i]);
        if(!abspath){
            die("Invalid input");
        }
        list_append(abspath, inputs);
    }

    if(list_get_elements_count(inputs) == 0){
        print_help();
        errno = EIO;
        die("No input to be processed has been specified");
    }
}

void collect_files(List *inputs){
    assert(inputs);
    int flags = 0;
    flags |= FTW_ACTIONRETVAL;
    if(!follow){
        flags |= FTW_PHYS;
    }
    ListIterator *iterator = list_iterator_new(inputs);
    while(list_iterator_has_next(iterator)){
        list_iterator_advance(iterator);
        char *path = list_iterator_get_element(iterator);
        int result = nftw(path, manage_entry, 20, flags);
        if(result == -1){
            die("Error in files collecting");
        }
    }
}

int manage_entry(const char *fpath, const struct stat *sb, int typeflag, struct FTW *ftbuf){
    if(typeflag == FTW_F){
        if(!list_contains(fpath, OptArgs.files_to_exclude)){
            list_append(fpath, files);
            return FTW_CONTINUE;
        }
    }
    if( (typeflag == FTW_D && recursive == true) || ftbuf->level == 0){
        return FTW_CONTINUE;
    } else {
        return FTW_SKIP_SUBTREE;
    }
    return FTW_STOP;
}

void initialize_global(){
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
    files = list_new();
    if(!files) die(NULL);
}

void exit_success(){
    free_global();
    exit(EXIT_SUCCESS);
}

void die(char *message){
    perror(message);
    free_global();
    exit(EXIT_FAILURE);
}

void free_global(){
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

List *get_words_from_file(const char *path){
    FILE *file = fopen(path, "r");
    if(!file){
        return NULL;
    }
    List *words = list_new();
    if(!words){
        return NULL;
    }
    char *buffer, *word;
    size_t lnsize = 0;
    int res = 0;
    while( (res = getline(&buffer, &lnsize, file)) > 0){
        if(res == -1){
            return NULL;
        }
        word = strtok(buffer, " ,.:;-_[]()/!£$%&?^|*€@#§°*'\n");
        while(word != NULL){
            res = list_append(word, words);
            if(res == -1){
                return NULL;
            }
            word = strtok(NULL, " ,.:;-_[]()/!£$%&?^|*€@#§°*'\n");
        }
    }
    return words;
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
    printf("\n\n");
}