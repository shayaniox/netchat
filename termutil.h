#ifndef _TERMUTIL_H
#define _TERMUTIL_H

#include <stdio.h>

#define tup(X) printf("\033[%dA", X)
#define tdown(X) printf("\033[%dB", X)
#define tclr() printf("\033[K")

#endif
