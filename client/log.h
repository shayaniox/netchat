#ifndef _LOG_H
#define _LOG_H

#include <stdio.h>

#define var(T, V) printf("\x1b[38;5;87m[VAR]\x1b[0m \x1b[1m" #V " :: " T "\x1b[0m\n", V);
#define info(M, ...) printf("\x1b[32m[INFO]\x1b[0m " M "\n", ##__VA_ARGS__);
#define error(M, ...) printf("\x1b[31m[ERROR]\x1b[0m " M "\n", ##__VA_ARGS__);

int log_file(const char *format, ...);

#endif
