
#ifndef CHAINING_H
#define CHAINING_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct Chaining_t Chaining;
typedef Chaining* Chaining_str;
#define CHAINING_STR_AFREE Chaining* __attribute__((__cleanup__(Chaining_free)))

struct Chaining_t {
    size_t size;
    char string[] __attribute__((__counted_by__(size)));
};

Chaining* Chaining_new(const char string[static 1]);
int Chaining_append(Chaining** c, const Chaining* string);
int Chaining_append_raw(Chaining *c[static 1], const char *string, size_t len);
Chaining* Chaining_clone(Chaining* c[static 1]);
void Chaining_print(const Chaining* c);
void Chaining_free(Chaining * c[static 1]);

#endif
