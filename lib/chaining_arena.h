#ifndef CHAINING_ARENA_H
#define CHAINING_ARENA_H

#include <stdlib.h>

typedef struct Chain_bucket_t_arena Chain_bucket_t;
typedef Chain_bucket_t* Chain_bucket;

struct Chain_bucket_t_arena {
    size_t capacity;
    size_t  size;
    char data[] __attribute__((__counted_by__(capacity)));
};

Chain_bucket Chain_bucket_new(size_t size);
void* Chain_bucket_alloc(Chain_bucket bucket, size_t size);
int Chain_bucket_realloc(Chain_bucket bucket, size_t size);
void Bucket_free(Chain_bucket b[static 1]);

#endif
