#ifndef _TUTIL_H
#define _TUTIL_H

#define up(N) printf("\033[%dA", N)
#define down(N) printf("\033[%dB", N)
#define left(N) printf("\033[%dD", N)
#define right(N) printf("\033[%dC", N)

#define clrline() printf("\033[2K")
#define clrscreen() printf("\033[2J")

#define srtaltbuf() printf("\033[?1049h")
#define qtaltbuf() printf("\033[?1049l")

#define pos0() printf("\033[J\033[H")

#define savestate() printf("\0337")
#define restorestate() printf("\0338")

int get_cursor_pos(int *row, int *column);
void move_cursor(int row, int column);
void scroll(int n);

#define shift(N) printf("\033[%dL", N)

#endif
