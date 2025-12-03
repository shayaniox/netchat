#include "tutil.h"
#include <stdio.h>
#include <sys/ioctl.h>
#include <termios.h>
#include <unistd.h>

int get_cursor_pos(int *row, int *column)
{
    struct termios prev, new;
    tcgetattr(STDIN_FILENO, &prev);
    new = prev;
    new.c_lflag &= ~(ECHO | ICANON);
    tcsetattr(STDIN_FILENO, TCSANOW, &new);

    printf("\033[6n");
    fflush(stdout);

    if (scanf("\033[%d;%dR", row, column) != 2) {
        tcsetattr(STDIN_FILENO, TCSAFLUSH, &prev);
        return -1;
    }

    tcsetattr(STDIN_FILENO, TCSANOW, &prev);

    return 0;
}

void move_cursor(int row, int column)
{
    printf("\033[%d;%dH", row, column);
    fflush(stdout);
}

void scroll(int n)
{
    for (int i = 0; i < n; i++)
        putchar('\n');
}
