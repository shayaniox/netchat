#include "plist.h"
#include <stddef.h>
#include <stdlib.h>
#include <sys/types.h>

struct plist *pl_init(size_t init_len)
{
    struct plist *pl = malloc(sizeof(struct plist));
    if (pl == NULL) return NULL;

    pl->len = 0;
    pl->cap = init_len;
    pl->data = calloc(init_len, sizeof(struct plist));
    if (pl->data == NULL) return NULL;

    return pl;
}

struct plist *pl_append(struct plist *pl, struct pollfd pfd)
{
    if (pl->len == pl->cap) {
        pl->cap *= 2;
        pl->data = realloc(pl->data, sizeof(struct plist) * pl->cap);
        if (pl->data == NULL) return NULL;
    }
    pl->data[pl->len] = pfd;
    pl->len++;

    return pl;
}

ssize_t pl_find(struct plist *pl, struct pollfd pfd)
{
    ssize_t index = -1;
    for (size_t i = 0; i < pl->len; i++)
        if (pl->data[i].fd == pfd.fd) {
            index = i;
            break;
        }
    return index;
}

struct plist *pl_remove(struct plist *pl, size_t index)
{
    return NULL;
}

void pl_free(struct plist *pl)
{
    if (pl == NULL) return;
    free(pl->data);
    free(pl);
}
