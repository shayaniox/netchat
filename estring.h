#ifndef _ESTRING_H
#define _ESTRING_H

#include <stddef.h>

typedef struct string {
    char *data;
    size_t len;
    size_t cap;
} *string_t;

string_t str_init();
int str_append(string_t str, char *chars, size_t l);
int str_clear(string_t str);
int str_free(string_t str);
int str_set(string_t str, char *chars, size_t l);

#endif
