#include "./chaining_arena.h"

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

void Bucket_free(Chain_bucket b[static 1]) {
    free(*b);
    *b = nullptr;
};
