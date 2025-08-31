#include "./chaining.h"
#include "chaining_arena.h"
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define VERIFY_ERR_1(statement, ...) if(statement) return __VA_ARGS__;

void Chaining_free(Chaining_str c[static 1]) {
    free(*c);
    *c = nullptr;
};


Chaining_str Chaining_look_for_retarena(Chain_bucket bucket, Chaining_str source, const char string[static 1], bool include_str) {
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

Chaining_str Chaining_look_for(Chaining_str source, const char string[static 1], bool include_str) {
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

bool Chaining_includes(Chaining_str source, const char string[static 1]) {
    const size_t len = strlen(string);
    size_t match_size = 0;

    if (source->size < len)
        return false;

    for (size_t i = 0; i < source->size; i++) {
        for (size_t j = 0; j < len; j++) {
            if (source->string[i + j] != string[j]) break;
            match_size++;
            if(match_size == len)
                return true;
        }
        match_size = 0;
    }

    return false;
}

Chaining_str Chaining_new_config (Chaining_str_config config, const char string[static 1]) {
    if (config.bucket && config.len > 0)
        return Chaining_new_arena_len(&config.bucket, string, config.len);

    if (config.bucket)
        return Chaining_new_arena(&config.bucket, string);

    if (config.len > 0)
        return Chaining_new_len(string, config.len);

    return Chaining_new(string);
}

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

Chaining_str Chaining_new_arena_len(Chain_bucket bucket[static 1], const char string[static 1], size_t len) {
    Chaining_str c = Chain_bucket_alloc(*bucket, sizeof(*c) + len);

    if(c == nullptr)
        return nullptr;

    *c = (Chaining) {
        .size = len
    };

    memcpy(c->string, string, len);
    return c;
};

Chaining_str Chaining_new(const char string[static 1]) {
    size_t len = strlen(string);
    Chaining_str c = malloc(sizeof(*c) + len);

    if(c == nullptr)
        return nullptr;

    *c = (Chaining) {
        .size = len
    };

    memcpy(c->string, string, len);
    return c;
}

Chaining_str Chaining_new_len(const char string[static 1], size_t len) {
    Chaining_str c = malloc(sizeof(*c) + len);

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

void Chaining_print(const Chaining_str c) {
    for (size_t i = 0; i < c->size; i++) {
        putchar(c->string[i]);
    }
    fflush(stdout);
}

void Chaining_println(const Chaining_str c) {
    for (size_t i = 0; i < c->size; i++) {
        putchar(c->string[i]);
    }
    putchar('\n');
    fflush(stdout);
}

Chaining_str Chaining_clone_arena(Chain_bucket bucket[static 1], Chaining_str c[static 1]) {
    Chaining_str clone = Chain_bucket_alloc(*bucket, sizeof(*clone) + (*c)->size);
    if(clone == nullptr)
        return nullptr;

    *clone = (Chaining) {
        .size = (*c)->size
    };
    memcpy(clone->string, (*c)->string, (*c)->size);
    return clone;
}

Chaining_str Chaining_clone(Chaining_str c[static 1]) {
    Chaining_str clone = malloc(sizeof(*clone) + (*c)->size);

    if(clone == nullptr)
        return nullptr;

    *clone = (Chaining) {
        .size = (*c)->size
    };
    memcpy(clone->string, (*c)->string, (*c)->size);

    return clone;
};

int Chaining_append_str_array(Chaining_str_array c[static 1], Chaining_str string) {
    if ((*c)->size + 2 >= (*c)->capacity) {
        (*c)->capacity *= 2;
        *c = realloc(*c, sizeof(**c) + sizeof(Chaining_str) * (*c)->capacity);

        if(*c == nullptr)
            return 1;
    }

    (*c)->array[(*c)->size] = string;
    (*c)->size++;
    return 0;
}

Chaining_str_array Chaining_new_array() {
    Chaining_str_array str_array = malloc(sizeof(Chaining_str_array) + sizeof(Chaining_str) * 4);
    if(str_array == nullptr)
        return nullptr;

    *str_array = (Chaining_array) {
        .capacity = 4,
        .size = 0
    };

    return str_array;
}

Chaining_str_array Chaining_explode(Chaining_str string, const char delimiters[static 1], bool strict) {
    size_t markers = 0;
    size_t match_size = 0;
    const size_t len = strlen(delimiters);
    Chaining_str_array str_array = Chaining_new_array();
    CHAINING_STR_AFREE buffer = Chaining_clone(&string);

    // aaa: aaa:11
    for (size_t i = 0; i < buffer->size; i++) {
        for (size_t j = 0; j < len; j++) {
            if(!strict)
                if (buffer->string[i] == delimiters[j]) {
                    markers++;
                    buffer->string[i] = '\0';
                    continue;
                };

            if (buffer->string[i + j] != delimiters[j]) break;
            match_size++;
            markers++;
            if(match_size == len)
                for (size_t k = 0; k < len; k++)
                    buffer->string[i + k] = '\0';
        }
        match_size = 0;
    }

    if (markers == 0) {
		VERIFY_ERR_1(Chaining_append_str_array(&str_array, Chaining_clone(&buffer)), nullptr);
		return str_array;
    }

    size_t count = 0;
    for (size_t i = 0; i < buffer->size; i++) {
        if (buffer->string[i] == '\0' && count > 0) {
            if (buffer->string[i-count] == '\0') {
                auto temp = CHAINING_STR_NEW(&buffer->string[i-count+1], .len = count);
                VERIFY_ERR_1(Chaining_append_str_array(&str_array, temp), nullptr);
            } else {
                auto temp = CHAINING_STR_NEW(&buffer->string[i-count], .len = count);
                VERIFY_ERR_1(Chaining_append_str_array(&str_array, temp), nullptr);
            }
            count = 0;
            continue;
        }
        count++;
		if (i+1 == buffer->size && count > 0) {
			auto temp = CHAINING_STR_NEW(&string->string[i+1-count], .len = count);
            VERIFY_ERR_1(Chaining_append_str_array(&str_array, temp), nullptr);
		}
    }

    return str_array;
}

Chaining_str_array Chaining_explode_in_bucket(Chain_bucket bucket, Chaining_str string, const char delimiters[static 1]) {
    size_t markers = 0;
    const size_t len = strlen(delimiters);
    Chaining_str_array str_array = Chaining_new_array();
    CHAINING_STR_AFREE buffer = Chaining_clone(&string);

    for (size_t i = 0; i < buffer->size; i++) {
        for (size_t j = 0; j < len; j++) {
            if (buffer->string[i] == delimiters[j]) {
                buffer->string[i] = '\0'; markers++;
            };
        }
    }

    if (markers == 0) {
		VERIFY_ERR_1(Chaining_append_str_array(&str_array, Chaining_clone_arena(&bucket, &buffer)), nullptr);
		return str_array;
    }

    size_t count = 0;
    for (size_t i = 0; i < buffer->size; i++) {
        if (buffer->string[i] == '\0' && count > 0) {
            if (buffer->string[i-count] == '\0') {
                Chaining_str temp = CHAINING_STR_NEW(&buffer->string[i-count+1], .len = count, .bucket = bucket);
                VERIFY_ERR_1(Chaining_append_str_array(&str_array, temp), nullptr);
            } else {
                Chaining_str temp = CHAINING_STR_NEW(&buffer->string[i-count], .len = count, .bucket = bucket);
                VERIFY_ERR_1(Chaining_append_str_array(&str_array, temp), nullptr);
            }
            count = 0;
            continue;
        }
        count++;
		if (i+1 == buffer->size && count > 0) {
			Chaining_str temp = CHAINING_STR_NEW(&string->string[i+1-count], .len = count, .bucket = bucket);
            VERIFY_ERR_1(Chaining_append_str_array(&str_array, temp), nullptr);
		}
    }

    return str_array;
}
