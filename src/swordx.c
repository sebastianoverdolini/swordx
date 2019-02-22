#define _GNU_SOURCE

#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <ctype.h>
#include <ftw.h>
#include <string.h>
#include <assert.h>
#include <time.h>
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
int manage_entry(const char *fpath, const struct stat *sb, int typeflag, struct FTW *ftbuf);
void collect_words(Trie *words, AVLTree *occurr_words);
int process_file(char *path, Trie *words, AVLTree *occurr_words, Trie *imported_words);
char *get_word(FILE *fp);
int write_log_line(char *logfilepath, char *name, int cw, int iw, double time);
int import_words(FILE *file, Trie *trie);
int save_word(char *word, Trie *words, AVLTree *occurr_words, Trie *imported_words);
void save_output(char *output_path, Trie *words, AVLTree *occurr_words);
int save_trie_on_file(char *filepath, Trie *trie);
bool word_is_valid(const char *word);
char *get_absolute_path(const char *path);
int convert_to_int(const char *text);
void initialize_global();
void free_global();
void exit_success();
void die(char *message);
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
    collect_words(words, occurr_words);
    save_output(OptArgs.output_path, words, occurr_words);

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
                FILE *file = fopen(optarg, "r");
                if(!file)
                    die("Invalid --ignore argument");
                char *word;
                while( (word = get_word(file)) != NULL){
                    if(word_is_valid(word)){
                        if( (trie_insert(word, OptArgs.words_to_ignore)) < 0){
                            die("Ignore fail arg");
                        }
                    }
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
    assert(files);
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


void collect_words(Trie *words, AVLTree *occurr_words){
    assert(words);
    assert(files);
    if(sortbyoccurrency)
        assert(occurr_words);
    Trie *imported_words = NULL;

    if(update){
        if( (imported_words = trie_new()) == NULL){
            die("Init fail");
        }
        if( (import_words(fopen(OptArgs.output_path, "r"), imported_words)) < 0){
            die("Fail with word import");
        }
    }
    ListIterator *files_iterator = list_iterator_new(files);
    while(list_iterator_has_next(files_iterator)){
        list_iterator_advance(files_iterator);
        char *file = list_iterator_get_element(files_iterator);
        if( (process_file(file, words, occurr_words, imported_words)) < 0 ){
            die("Fail with file processing");
        }
    }
    list_iterator_destroy(files_iterator);
}

int import_words(FILE *file, Trie *trie){
    if(!file){
        return -1;
    }
    assert(trie);
    char *word;
    while( (word = get_word(file)) != NULL){
        if(word_is_valid(word)){
            if(trie_insert(word, trie) < 0)
                return -1;
            else
                if( (word = get_word(file)) == NULL)
                    return -1;
        }
    }
    fclose(file);
    return 0;
}

int process_file(char *path, Trie *words, AVLTree *occurr_words, Trie *imported_words){
    assert(words);
    if(sortbyoccurrency)
        assert(occurr_words);
    if(update)
        assert(imported_words);
    int words_count = 0, words_valid = 0, words_ignored = 0;
    clock_t begin = clock();
    char *word;
    FILE *file = fopen(path, "r");
    if(!file)
        return -1;
    while( (word = get_word(file)) != NULL){
        words_count++;
        if(word_is_valid(word)){
            if(!update || trie_contains(word, imported_words)){
                if( (save_word(word, words, occurr_words, imported_words) < 0))
                    return -1;
                words_valid++;
            }
        }
    }
    fclose(file);
    clock_t end = clock();
    double time_spent = (double) (end-begin) / CLOCKS_PER_SEC;
    words_ignored = words_count - words_valid;
    if(log){
        if(write_log_line(OptArgs.log_path, path, words_valid, words_ignored,time_spent) < 0){
            return -1;
        }
    }
    return 0;
}

int save_word(char *word, Trie *words, AVLTree *occurr_words, Trie *imported_words){
    assert(words);
    assert(occurr_words);
    if(update)
        assert(imported_words);
    if (sortbyoccurrency){
        int old_occ = trie_get_word_occurrences(word, words);
        if (old_occ != 0)
            trie_remove(word, avltree_get_element_by_key(old_occ, occurr_words));
        if (!avltree_contains_key(old_occ + 1, occurr_words))
            if (avltree_insert(old_occ + 1, trie_new(), occurr_words) < 0)
                return -1;
        Trie *occ_trie = avltree_get_element_by_key(old_occ + 1, occurr_words);
        if (trie_insert_with_occ(word, old_occ + 1, occ_trie) < 0)
            return -1;
    }
    if ((trie_insert(word, words) < 0))
        return -1;
    return 0;
}

char *get_word(FILE *fp){
    char word[200];
    int ch, i=0;

    while(EOF != (ch=fgetc(fp)) && isblank(ch) && isspace(ch))
        ;
    if(ch == EOF)
        return NULL;
    do{
        word[i++] = tolower(ch);
    }while(EOF!=(ch=fgetc(fp)) && !isblank(ch) && !isspace(ch));

    word[i]='\0';
    return strdup(word);
}

int write_log_line(char *logfilepath, char *name, int cw, int iw, double time){
    FILE *logfile = fopen(logfilepath, "a");
    if (!logfile)
        return -1;
    fprintf(logfile, "%s;%d;%d;%lf\n", name, cw, iw, time);
    fclose(logfile);
    return 0;
}

void save_output(char *output_path, Trie *words, AVLTree *occurr_words){
    assert(occurr_words);
    assert(words);
    int res = 0;
    if(sortbyoccurrency){
        AVLTreeIterator *avliterator = avltree_iterator_new(occurr_words);
        while(avltree_iterator_has_next(avliterator)){
            avltree_iterator_advance(avliterator);
            res = save_trie_on_file(output_path, avltree_iterator_get_element(avliterator));
            if(res < 0){
                die("Error in output file");
            }
        }
        avltree_iterator_destroy(avliterator);
    } else {
        res = save_trie_on_file(output_path, words);
        if(res < 0){
            die("Error in output file");
        }
    }
}

int save_trie_on_file(char *filepath, Trie *trie){
    char *write_mode = (sortbyoccurrency) ? "a" : "w";
    int res = 0;
    FILE *file = fopen(filepath, write_mode);
    if(!file){
        return -1;
    }
    List *wordlist = trie_get_wordlist(trie);
    ListIterator *wl_iterator = list_iterator_new(wordlist);
    while(list_iterator_has_next(wl_iterator)){
        list_iterator_advance(wl_iterator);
        res = fprintf(file, "%s\n", list_iterator_get_element(wl_iterator));
        if(res < 0) return -1;
    }
    return 0;
}

bool word_is_valid(const char *word){
    if(!word){
        return false;
    }
    if(strlen(word) < OptArgs.minimum_word_length){
        return false;
    }
    for(int i = 0; i < strlen(word); i++){
        if(!isalnum(word[i]))
            return false;
        if(alpha && isdigit(word[i]))
            return false;
    }
    if(trie_contains(word, OptArgs.words_to_ignore)){
        return false;
    }
    return true;
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

void free_global(){
    list_destroy(OptArgs.files_to_exclude);
    trie_destroy(OptArgs.words_to_ignore);
    free(OptArgs.output_path);
    free(OptArgs.log_path);
    list_destroy(files);
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