#ifndef CHAINING_ARRAY_H
#define CHAINING_ARRAY_H
#include "./chaining.h"

#define VERIFY_ERR_1(statement, ...) if(statement) return __VA_ARGS__;
#define CHAIN_EXPLODE(destination, string, delimiters, ...) Chaining_explode_config((Chaining_explode_config_t) {__VA_ARGS__}, destination, string, delimiters)

typedef struct Chaining_t_array Chaining_array;
typedef Chaining_array* Chaining_str_array;
typedef struct Chaining_explode_config_t Chaining_explode_config_t;


struct Chaining_explode_config_t {
    bool strict;
    Chain_bucket bucket;
};

struct Chaining_t_array {
    size_t capacity;
    size_t size;
    chainstr array[] __attribute__((__counted_by__(capacity)));
};

int Chaining_append_array(Chaining_str_array c[static 1], chainstr string);
Chaining_str_array Chaining_new_array();

int Chaining_explode_config(Chaining_explode_config_t config, Chaining_str_array destination[static 1], chainstr string, const char delimiters[static 1]);
Chaining_str_array Chaining_explode(chainstr string, const char delimiters[static 1], bool strict);
int Chaining_explode_in_bucket(Chain_bucket bucket, chainstr string, Chaining_str_array destination[static 1], const char delimiters[static 1], bool strict) ;

#endif
