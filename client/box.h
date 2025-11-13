#ifndef _BOX_H
#define _BOX_H

#include "estring.h"
#include <stddef.h>

#define UR "╮"
#define UL "╭"
#define LR "╯"
#define LL "╰"
#define V "│"
#define H "─"

struct box {
    int row;
    int column;
    string_t text;
};

struct box *box_init(int row, int column, string_t text);
void box_draw(struct box *);
void box_addcolumn(struct box *b, char ch);
void box_done(struct box *b);
void box_new(struct box *b);

#endif
