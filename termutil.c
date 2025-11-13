#include "termutil.h"
#include <stdio.h>
#include <termios.h>
#include <unistd.h>

int get_cursor_pos(int *row, int *column)
{
    struct termios prev, new;
    tcgetattr(STDIN_FILENO, &prev);
    new = prev;
    new.c_iflag &= ~(ECHO | ICANON);
    tcsetattr(STDIN_FILENO, TCSAFLUSH, &new);

    printf("\033[6n");
    fflush(stdout);

    if (scanf("\033[%d;%dR", row, column) != 2) {
        tcsetattr(STDIN_FILENO, TCSAFLUSH, &prev);
        return -1;
    }

    tcsetattr(STDIN_FILENO, TCSAFLUSH, &prev);

    return 0;
}

int move_line(int from_row, int to_row)
{

    return -1;
}
