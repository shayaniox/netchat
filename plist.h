#ifndef _PLIST_H
#define _PLIST_H

#include <stddef.h>
#include <sys/poll.h>
#include <sys/types.h>

struct plist {
    struct pollfd *data;
    size_t len;
    size_t cap;
};

struct plist *pl_init(size_t init_len);
struct plist *pl_append(struct plist *pl, struct pollfd pfd);
ssize_t pl_find(struct plist *pl, struct pollfd pfd);
struct plist *pl_remove(struct plist *pl, size_t index);
void pl_free(struct plist *pl);

#endif
