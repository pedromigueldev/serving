
#ifndef CHAINING_H
#define CHAINING_H

#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "./chaining_arena.h"

typedef struct Chaining_str_config_t Chaining_str_config;
typedef struct Chaining_t Chaining;
typedef struct Chaining_t_array Chaining_array;
typedef Chaining* Chaining_str;
typedef Chaining_array* Chaining_str_array;

#define CHAINING_STR_AFREE Chaining_str __attribute__((__cleanup__(Chaining_free)))
#define CHAINING_STR_NEW(string, ...) Chaining_new_config((Chaining_str_config) {__VA_ARGS__}, string)

struct Chaining_t {
    size_t size;
    char string[] __attribute__((__counted_by__(size)));
};

struct Chaining_t_array {
    size_t capacity;
    size_t size;
    Chaining_str array[] __attribute__((__counted_by__(capacity)));
};

struct Chaining_str_config_t {
    size_t len;
    Chain_bucket bucket;

};
int Chaining_append_str_array(Chaining_str_array c[static 1], Chaining_str string);
Chaining_str_array Chaining_explode(Chaining_str string, const char delimiters[static 1], bool strict);
Chaining_str_array Chaining_explode_in_bucket(Chain_bucket bucket, Chaining_str string, const char delimiters[static 1]);
Chaining_str_array Chaining_new_array();

Chaining_str Chaining_look_for_retarena(Chain_bucket bucket, Chaining_str source, const char string[static 1], bool include_str);
Chaining_str Chaining_look_for(Chaining_str source, const char string[static 1], bool include_str);
bool Chaining_includes(Chaining_str source, const char string[static 1]);

Chaining_str Chaining_new_config (Chaining_str_config config, const char string[static 1]);
Chaining_str Chaining_new(const char string[static 1]);
Chaining_str Chaining_new_len(const char string[static 1], size_t len);
Chaining_str Chaining_new_arena(Chain_bucket bucket[static 1], const char string[static 1]);
Chaining_str Chaining_new_arena_len(Chain_bucket bucket[static 1], const char string[static 1], size_t len);

int Chaining_append_raw(Chaining *c[static 1], const char *string, size_t len);
int Chaining_append_raw_arena(Chain_bucket bucket[static 1], Chaining *c[static 1], const char *string, size_t len);

Chaining_str Chaining_clone(Chaining_str c[static 1]);
Chaining_str Chaining_clone_arena(Chain_bucket bucket[static 1], Chaining_str c[static 1]);

void Chaining_print(const Chaining_str c);
void Chaining_println(const Chaining_str c);
void Chaining_free(Chaining_str c[static 1]);

#endif
