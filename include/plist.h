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
int pl_remove(struct plist *pl, struct pollfd pfd);
void pl_free(struct plist *pl);

#endif
