
#ifndef CHAINING_H
#define CHAINING_H

#include <limits.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "./chaining_arena.h"

typedef struct Chaining_str_config_t Chaining_str_config;
typedef struct Chaining_t Chaining;
typedef Chaining* chainstr;

#define CHAINING_STR_AFREE chainstr __attribute__((__cleanup__(Chaining_free)))
#define CHAINING_STR_NEW(string, ...) Chaining_new_config((Chaining_str_config) {__VA_ARGS__}, string)

struct Chaining_t {
    uint16_t size;
    char string[] __attribute__((__counted_by__(size)));
};

struct Chaining_str_config_t {
    size_t len;
    Chain_bucket bucket;
};

chainstr Chaining_look_for_retarena(Chain_bucket bucket, chainstr source, const char string[static 1], bool include_str);
chainstr Chaining_look_for(chainstr source, const char string[static 1], bool include_str);
bool Chaining_includes(chainstr source, const char string[static 1]);

chainstr Chaining_new_config (Chaining_str_config config, const char string[static 1]);
chainstr Chaining_new(const char string[static 1]);
chainstr Chaining_new_len(const char string[static 1], size_t len);
chainstr Chaining_new_arena(Chain_bucket bucket[static 1], const char string[static 1]);
chainstr Chaining_new_arena_len(Chain_bucket bucket[static 1], const char string[static 1], size_t len);

int Chaining_append_raw(Chaining *c[static 1], const char *string, size_t len);
int Chaining_append_raw_arena(Chain_bucket bucket[static 1], Chaining *c[static 1], const char *string, size_t len);

chainstr Chaining_clone(chainstr c[static 1]);
chainstr Chaining_clone_arena(Chain_bucket bucket[static 1], chainstr c[static 1]);

void Chaining_print(const chainstr c);
void Chaining_println(const chainstr c);
void Chaining_free(chainstr c[static 1]);

#endif
