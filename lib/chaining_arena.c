#include "./chaining_arena.h"
#include <stddef.h>
#include <stdlib.h>

#define PAGE_SIZE 4096
#define VERIFY_ERR_1(statement, ...) if(statement) return __VA_ARGS__

Chain_bucket Chain_bucket_new(size_t size) {
    size_t allocate = 0;

    if (size < PAGE_SIZE) {
        allocate = PAGE_SIZE;
    } else {
        allocate = size + PAGE_SIZE;
    }

    Chain_bucket b = malloc(sizeof(*b) + allocate);

    if(b == nullptr)
        return nullptr;

    *b = (Chain_bucket_t) {
        .capacity = allocate
    };

    return b;
};

void* Chain_bucket_alloc(Chain_bucket bucket[static 1], size_t size) {

    if((*bucket)->size + size >= (*bucket)->capacity) {
        VERIFY_ERR_1(Chain_bucket_realloc(bucket, (*bucket)->size + size + PAGE_SIZE), nullptr);
    }

    void *ptr = (*bucket)->data + (*bucket)->size;

    if(ptr == nullptr)
        return nullptr;

    (*bucket)->size += size;

    return ptr;
};

int Chain_bucket_realloc(Chain_bucket bucket[static 1], size_t size) {
    *bucket = realloc(*bucket, sizeof(**bucket) + size);

    if(*bucket == nullptr)
        return 1;

    (*bucket)->capacity = size;
    return 0;
}

void Bucket_free(Chain_bucket b[static 1]) {
    free(*b);
    *b = nullptr;
};
