#include "list.h"
#include <stdio.h>
#include <stdlib.h>

const size_t DEFAULT_CAP = 4;

struct list *l_init(size_t init_len)
{
    struct list *l = malloc(sizeof(struct list));
    if (l == NULL) return NULL;

    l->len = 0;
    l->cap = init_len ? init_len : DEFAULT_CAP;

    l->data = calloc(l->cap, sizeof(int));
    if (l->data == NULL) return NULL;

    return l;
}

struct list *l_append(struct list *l, int num)
{
    if (l->len == l->cap) {
        l->cap *= 2;
        l->data = realloc(l->data, sizeof(int) * l->cap);
        if (l->data == NULL) return NULL;
    }
    l->data[l->len] = num;
    l->len++;
    return l;
}

void l_free(struct list *l)
{
    if (l == NULL) return;
    free(l->data);
    free(l);
}
