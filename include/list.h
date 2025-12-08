#ifndef _LIST_H
#define _LIST_H

#include <stddef.h>
#include <stdio.h>

struct list {
    int *data;
    size_t len;
    size_t cap;
};

struct list *l_init(size_t init_len);
struct list *l_append(struct list *l, int num);
void l_free(struct list *l);

#endif
