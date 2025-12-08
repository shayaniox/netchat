#include "estring.h"
#include <errno.h>
#include <stdlib.h>
#include <string.h>

#define INITIAL_CAPACITY 16

string_t str_init()
{
    // TODO: Use chars and l for initialization
    string_t str = malloc(sizeof(struct string));
    if (!str) {
        return NULL;
    }

    str->data = malloc(INITIAL_CAPACITY);
    if (!str->data) {
        free(str);
        return NULL;
    }

    str->data[0] = '\0';
    str->len = 0;
    str->cap = INITIAL_CAPACITY;

    return str;
}

int str_append(string_t str, char *chars, size_t l)
{
    if (!str || !chars) {
        return -1;
    }

    // Check if we need to resize
    size_t required = str->len + l + 1; // +1 for null terminator
    if (required > str->cap) {
        // Grow capacity (double until it fits)
        size_t new_cap = str->cap;
        while (new_cap < required) {
            new_cap *= 2;
        }

        char *new_data = realloc(str->data, new_cap);
        if (!new_data) {
            return -1; // Allocation failed
        }

        str->data = new_data;
        str->cap = new_cap;
    }

    // Append the characters
    memcpy(str->data + str->len, chars, l);
    str->len += l;
    str->data[str->len] = '\0';

    return 0; // Success
}

int str_clear(string_t str)
{
    memset(str->data, 0, str->len);
    str->len = 0;

    return 0; // Success
}

int str_free(string_t str)
{
    if (!str) {
        return -1;
    }

    free(str->data);
    free(str);

    return 0; // Success
}

int str_set(string_t str, char *chars, size_t l)
{
    if (str == NULL) {
        return EINVAL;
    }

    if (l >= str->cap) {
        str->cap += l + 1;
        str->data = realloc(str->data, l + 1);
        if (str->data == NULL) return -1;
    }
    str->len = l;
    memcpy(str->data, chars, l);
    str->data[str->len] = '\0';

    return 0;
}
