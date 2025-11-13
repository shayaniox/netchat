#ifndef _TERMUTIL_H
#define _TERMUTIL_H

#include <stdio.h>

struct position {
    int row;
    int column;
};

#define tup(X) printf("\033[%dA", X)
#define tdown(X) printf("\033[%dB", X)
#define tclr() printf("\033[K")

int get_cursor_pos(int *row, int *column);
int move_line(int from_row, int to_row);

#endif
