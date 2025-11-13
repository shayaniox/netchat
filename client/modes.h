#ifndef _MODES_H
#define _MODES_H

#include <termios.h>

int ttySetCbreak(int fd, struct termios *current_tty);
int ttySetRaw(int fd, struct termios *current_tty);

#endif
