#include "./chaining.h"
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void Chaining_free(Chaining * c[static 1]) {
    free(*c);
    *c = nullptr;
};
Chaining* Chaining_new(const char string[static 1]) {
    Chaining* c = malloc(sizeof(*c) + strlen(string));

    if(c == nullptr)
        return nullptr;

    *c = (Chaining) {
        .size = strlen(string)
    };

    memcpy(c->string, string, strlen(string));
    return c;
}

int Chaining_append_raw(Chaining *c[static 1], const char *string, size_t len) {
    size_t new_size = (*c)->size + len;

    *c = realloc(*c, sizeof(**c) + new_size + 1);
    if(*c == nullptr)
        return 1;

    memcpy((*c)->string + (*c)->size, string, len);
    (*c)->size = new_size + 1;
    (*c)->string[new_size] = '\0';

    return 0;
}

void Chaining_print(const Chaining* c) {
    for (size_t i = 0; i < c->size; i++) {
        putchar(c->string[i]);
    }
}


Chaining* Chaining_clone(Chaining* c[static 1]) {
    Chaining* clone = malloc(sizeof(*clone) + (*c)->size);

    if(clone == nullptr)
        return nullptr;

    *clone = (Chaining) {
        .size = (*c)->size
    };
    memcpy(clone->string, (*c)->string, (*c)->size);

    return clone;
};
