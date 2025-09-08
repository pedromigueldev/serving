#include "./chaining.h"
#include "chaining_arena.h"
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void Chaining_free(chainstr c[static 1]) {
    free(*c);
    *c = nullptr;
};


chainstr Chaining_look_for_retarena(Chain_bucket bucket, chainstr source, const char string[static 1], bool include_str) {
    const size_t len = strlen(string);
    size_t match_size = 0;

    if (source->size < len)
        return nullptr;

    for (size_t i = 0; i < source->size; i++) {
        for (size_t j = 0; j < len; j++) {
            if (source->string[i + j] != string[j]) break;
            match_size++;
            if(match_size == len) {
                if (include_str)
                    return CHAINING_STR_NEW(&source->string[i], .bucket = bucket, .len = source->size - i);
                if(source->size - i - len > 0)
                    return CHAINING_STR_NEW(&source->string[i + len], .bucket = bucket, .len = source->size - i - len);
                else
                    return CHAINING_STR_NEW("", .bucket = bucket);
            }
        }
        match_size = 0;
    }

    return nullptr;
}

chainstr Chaining_look_for(chainstr source, const char string[static 1], bool include_str) {
    const size_t len = strlen(string);
    size_t match_size = 0;

    if (source->size < len)
        return nullptr;

    for (size_t i = 0; i < source->size; i++) {
        for (size_t j = 0; j < len; j++) {
            if (source->string[i + j] != string[j]) break;
            match_size++;
            if(match_size == len) {
                if (include_str)
                    return Chaining_new_len(&source->string[i], source->size - i);
                if(source->size - i - len > 0)
                    return Chaining_new_len(&source->string[i + len], source->size - i - len);
                else
                    return CHAINING_STR_NEW("");
            }
        }
        match_size = 0;
    }

    return nullptr;
}

bool Chaining_includes(chainstr source, const char string[static 1]) {
    const size_t len = strlen(string);
    size_t match_size = 0;

    if (source->size < len)
        return false;

    for (size_t i = 0; i < source->size; i++) {
        for (size_t j = 0; j < len; j++) {
            if(i + j >= source->size)
                break;
            if (source->string[i + j] != string[j]) break;
            match_size++;
            if(match_size == len)
                return true;
        }
        match_size = 0;
    }

    return false;
}

chainstr Chaining_new_config (Chaining_str_config config, const char string[static 1]) {
    if (config.bucket && config.len > 0)
        return Chaining_new_arena_len(&config.bucket, string, config.len);

    if (config.bucket)
        return Chaining_new_arena(&config.bucket, string);

    if (config.len > 0)
        return Chaining_new_len(string, config.len);

    return Chaining_new(string);
}

chainstr Chaining_new_arena(Chain_bucket bucket[static 1], const char string[static 1]) {
    size_t len = strlen(string);
    chainstr c = Chain_bucket_alloc(bucket, sizeof(*c) + len);

    if(c == nullptr)
        return nullptr;

    *c = (Chaining) {
        .size = len
    };

    memcpy(c->string, string, len);
    return c;
};

chainstr Chaining_new_arena_len(Chain_bucket bucket[static 1], const char string[static 1], size_t len) {
    chainstr c = Chain_bucket_alloc(bucket, sizeof(*c) + len);

    if(c == nullptr)
        return nullptr;

    *c = (Chaining) {
        .size = len
    };

    memcpy(c->string, string, len);
    return c;
};

chainstr Chaining_new(const char string[static 1]) {
    size_t len = strlen(string);
    chainstr c = malloc(sizeof(*c) + len);

    if(c == nullptr)
        return nullptr;

    *c = (Chaining) {
        .size = len
    };

    memmove(c->string, string, len);
    return c;
}

chainstr Chaining_new_len(const char string[static 1], size_t len) {
    chainstr c = malloc(sizeof(*c) + len);

    if(c == nullptr)
        return nullptr;

    *c = (Chaining) {
        .size = len
    };

    memcpy(c->string, string, len);
    return c;
}

int Chaining_append_raw_arena(Chain_bucket bucket[static 1], Chaining *c[static 1], const char *string, size_t len) {
    if (len == 0)
        return 1;

    CHAINING_STR_AFREE temp_copy = Chaining_clone(c);
    size_t new_size = temp_copy->size + len;

    chainstr new_string = Chain_bucket_alloc(bucket, sizeof(chainstr) + new_size);

    if(new_string == nullptr)
        return 1;

    new_string->size = new_size;
    memmove(new_string->string, temp_copy->string, temp_copy->size);
    memmove(new_string->string + temp_copy->size, string, len);

    *c = new_string;
    return 0;
}

int Chaining_append_raw(Chaining *c[static 1], const char *string, size_t len) {
    size_t new_size = (*c)->size + len;

    *c = realloc(*c, sizeof(chainstr) + new_size);
    if(*c == nullptr)
        return 1;

    memmove((*c)->string + (*c)->size, string, len);
    (*c)->size = new_size;

    return 0;
}

void Chaining_print(const chainstr c) {
    for (size_t i = 0; i < c->size; i++) {
        putchar(c->string[i]);
    }
    fflush(stdout);
}

void Chaining_println(const chainstr c) {
    for (size_t i = 0; i < c->size; i++) {
        putchar(c->string[i]);
    }
    putchar('\n');
    fflush(stdout);
}

chainstr Chaining_clone_arena(Chain_bucket bucket[static 1], chainstr c[static 1]) {
    chainstr clone = Chain_bucket_alloc(bucket, sizeof(*clone) + (*c)->size);
    if(clone == nullptr)
        return nullptr;

    *clone = (Chaining) {
        .size = (*c)->size
    };
    memcpy(clone->string, (*c)->string, (*c)->size);
    return clone;
}

chainstr Chaining_clone(chainstr c[static 1]) {
    chainstr clone = malloc(sizeof(*clone) + (*c)->size);

    if(clone == nullptr)
        return nullptr;

    *clone = (Chaining) {
        .size = (*c)->size
    };
    memcpy(clone->string, (*c)->string, (*c)->size);

    return clone;
};
