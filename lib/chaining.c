#include "./chaining.h"
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void Chaining_free(Chaining_str c[static 1]) {
    free(*c);
    *c = nullptr;
};

void Bucket_free(Chain_bucket b[static 1]) {
    free(*b);
    *b = nullptr;
};

Chain_bucket Chain_bucket_new(size_t size) {
    Chain_bucket b = malloc(sizeof(*b) + size);

    if(b == nullptr)
        return nullptr;

    *b = (Chain_bucket_t) {
        .size = size
    };

    return b;
};

void* Chain_bucket_alloc(Chain_bucket bucket, size_t size) {
    void *ptr = bucket->data + bucket->capacity;

    if(ptr == nullptr)
        return nullptr;

    bucket->capacity += size;
    return ptr;
};

Chaining_str Chaining_new_arena(Chain_bucket bucket[static 1], const char string[static 1]) {
    size_t len = strlen(string);
    Chaining_str c = Chain_bucket_alloc(*bucket, sizeof(*c) + len);

    if(c == nullptr)
        return nullptr;

    *c = (Chaining) {
        .size = len
    };

    memcpy(c->string, string, len);
    return c;
};

Chaining* Chaining_new(const char string[static 1]) {
    size_t len = strlen(string);
    Chaining* c = malloc(sizeof(*c) + len);

    if(c == nullptr)
        return nullptr;

    *c = (Chaining) {
        .size = len
    };

    memcpy(c->string, string, len);
    return c;
}

int Chaining_append_raw_arena(Chain_bucket bucket[static 1], Chaining *c[static 1], const char *string, size_t len) {
    size_t new_size = (*c)->size + len;

    Chaining_str new_string = Chain_bucket_alloc(*bucket, sizeof(**c) + new_size);
    if(new_string == nullptr)
        return 1;

    memcpy(new_string->string, (*c)->string, (*c)->size);
    memcpy(new_string->string + (*c)->size, string, len);

    new_string->size = new_size;
    *c = new_string;
    return 0;
}

int Chaining_append_raw(Chaining *c[static 1], const char *string, size_t len) {
    size_t new_size = (*c)->size + len;

    *c = realloc(*c, sizeof(**c) + new_size);
    if(*c == nullptr)
        return 1;

    memcpy((*c)->string + (*c)->size, string, len);
    (*c)->size = new_size;

    return 0;
}

void Chaining_print(const Chaining* c) {
    for (size_t i = 0; i < c->size; i++) {
        putchar(c->string[i]);
    }
}

Chaining_str Chaining_clone_arena(Chain_bucket bucket[static 1], Chaining* c[static 1]) {
    Chaining_str clone = Chain_bucket_alloc(*bucket, sizeof(*clone) + (*c)->size);
    if(clone == nullptr)
        return nullptr;

    *clone = (Chaining) {
        .size = (*c)->size
    };
    memcpy(clone->string, (*c)->string, (*c)->size);
    return clone;
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
