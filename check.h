#ifndef _CHECK_H
#define _CHECK_H

#include "errfunc.h"

#define assert(X, M, ...) \
    if (!(X))             \
    err_exit(M, ##__VA_ARGS__)

#endif
