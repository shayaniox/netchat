#ifndef _CHECK_H
#define _CHECK_H

#include "log.h"

#define assert(X, M, ...) \
    if (!(X))             \
    err_exit(M, ##__VA_ARGS__)

#define check(X, M, ...)         \
    if (!(X)) {                  \
        error(M, ##__VA_ARGS__); \
        return -1;               \
    }

#endif
