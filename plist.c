#include "plist.h"
#include <errno.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>

struct plist *pl_init(size_t init_len)
{
    struct plist *pl = malloc(sizeof(struct plist));
    if (pl == NULL) return NULL;

    pl->len = 0;
    pl->cap = init_len;
    pl->data = calloc(init_len, sizeof(struct pollfd));
    if (pl->data == NULL) return NULL;

    return pl;
}

struct plist *pl_append(struct plist *pl, struct pollfd pfd)
{
    if (pl->len == pl->cap) {
        pl->cap *= 2;
        pl->data = realloc(pl->data, sizeof(struct pollfd) * pl->cap);
        if (pl->data == NULL) return NULL;
    }
    pl->data[pl->len] = pfd;
    pl->len++;

    return pl;
}

static ssize_t pl_find(struct plist *pl, struct pollfd pfd)
{
    ssize_t index = -1;
    for (size_t i = 0; i < pl->len; i++)
        if (pl->data[i].fd == pfd.fd) {
            index = i;
            break;
        }
    return index;
}

int pl_remove(struct plist *pl, struct pollfd pfd)
{
    ssize_t index = pl_find(pl, pfd);
    if (index == -1) {
        errno = EINVAL;
        return -1;
    }
    size_t count = (pl->len - 1) - index;
    if (count && memcpy(pl->data + index, pl->data + index + 1, sizeof(struct pollfd) * count) == NULL)
        return -1;
    pl->len--;
    memset(pl->data + pl->len, 0, sizeof(struct pollfd));
    return 0;
}

void pl_free(struct plist *pl)
{
    if (pl == NULL) return;
    free(pl->data);
    free(pl);
}
