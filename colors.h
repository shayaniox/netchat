#ifndef _COLORS_H
#define _COLORS_H

#include <stdio.h>

#define nc "\033[0m"
#define green "\033[32m"
#define blue "\033[34m"
#define color(N)         \
    do {                 \
        printf("%s", N); \
        fflush(stdout);  \
    } while (0)

#endif
