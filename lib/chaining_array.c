#include "./chaining_array.h"

int Chaining_append_array(Chaining_str_array c[static 1], Chaining_str string) {

    if (*c == nullptr) {
        *c = malloc(sizeof(Chaining_str_array) + sizeof(Chaining_str) * 4);
        if(*c == nullptr)
            return 1;

        **c = (Chaining_array) {
            .capacity = 4,
            .size = 0
        };
    }

    if ((*c)->size + 2 >= (*c)->capacity) {
        (*c)->capacity *= 2;
        *c = realloc(*c, sizeof(Chaining_str_array) + sizeof(Chaining_str) * (*c)->capacity);
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
    Chaining_str_array str_array = {0};
    CHAINING_STR_AFREE buffer = Chaining_clone(&string);
    const size_t len = strlen(delimiters);

    for (size_t i = 0; i < buffer->size; i++) {
        for (size_t j = 0; j < len; j++) {
            if(!strict) {
                if (buffer->string[i] == delimiters[j]) {
                    markers++;
                    buffer->string[i] = '\0';
                    continue;
                };
                continue;
            }

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
		VERIFY_ERR_1(Chaining_append_array(&str_array, Chaining_clone(&buffer)), nullptr);
		return str_array;
    }

    size_t count = 0;
    for (size_t i = 0; i < buffer->size; i++) {
        if (buffer->string[i] == '\0' && count > 0) {
            if (buffer->string[i-count] == '\0') {
                auto temp = CHAINING_STR_NEW(&buffer->string[i-count+1], .len = count);
                VERIFY_ERR_1(Chaining_append_array(&str_array, temp), nullptr);
            } else {
                auto temp = CHAINING_STR_NEW(&buffer->string[i-count], .len = count);
                VERIFY_ERR_1(Chaining_append_array(&str_array, temp), nullptr);
            }
            count = 0;
            continue;
        }
        count++;
		if (i+1 == buffer->size && count > 0) {
			auto temp = CHAINING_STR_NEW(&string->string[i+1-count], .len = count);
            VERIFY_ERR_1(Chaining_append_array(&str_array, temp), nullptr);
		}
    }

    return str_array;
}

int Chaining_explode_in_bucket(Chain_bucket bucket, Chaining_str string, Chaining_str_array destination[static 1], const char delimiters[static 1], bool strict) {
    size_t markers = 0;
    size_t match_size = 0;
    CHAINING_STR_AFREE buffer = Chaining_clone(&string);
    const size_t len = strlen(delimiters);

    for (size_t i = 0; i < buffer->size; i++) {
        for (size_t j = 0; j < len; j++) {
            if(!strict) {
                if (buffer->string[i] == delimiters[j]) {
                    markers++;
                    buffer->string[i] = '\0';
                    break;
                };
                continue;
            }

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
		return Chaining_append_array(destination, Chaining_clone_arena(&bucket, &buffer));
    }

    size_t count = 0;
    for (size_t i = 0; i < buffer->size; i++) {
        auto debug = buffer->string[i];
        if (debug == '\0' && count > 0) {
            Chaining_str temp = CHAINING_STR_NEW(&buffer->string[i-count], .len = count, .bucket = bucket);
            VERIFY_ERR_1(Chaining_append_array(destination, temp), 1);
            count = 0;
            continue;
        } else if (debug == '\0' && count == 0) continue;
        count++;
    }

    return 0;
}
