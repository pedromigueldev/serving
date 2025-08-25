
#ifndef CHAINING_H
#define CHAINING_H

#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct Chaining_t Chaining;
typedef struct Chain_bucket_t_arena Chain_bucket_t;

typedef Chain_bucket_t* Chain_bucket;
typedef Chaining* Chaining_str;

#define CHAINING_STR_AFREE Chaining* __attribute__((__cleanup__(Chaining_free)))

struct Chaining_t {
    size_t size;
    char string[] __attribute__((__counted_by__(size)));
};

struct Chain_bucket_t_arena {
    size_t capacity;
    size_t  size;
    char data[] __attribute__((__counted_by__(size)));
};

Chain_bucket Chain_bucket_new(size_t size);
void* Chain_bucket_alloc(Chain_bucket bucket, size_t size);

Chaining* Chaining_new(const char string[static 1]);
Chaining_str Chaining_new_arena(Chain_bucket bucket[static 1], const char string[static 1]);

int Chaining_append(Chaining** c, const Chaining* string);
int Chaining_append_raw(Chaining *c[static 1], const char *string, size_t len);
int Chaining_append_raw_arena(Chain_bucket bucket[static 1], Chaining *c[static 1], const char *string, size_t len);

Chaining* Chaining_clone(Chaining* c[static 1]);
Chaining_str Chaining_clone_arena(Chain_bucket bucket[static 1], Chaining* c[static 1]);

void Chaining_print(const Chaining* c);

void Chaining_free(Chaining_str c[static 1]);
void Bucket_free(Chain_bucket b[static 1]) ;

#endif
