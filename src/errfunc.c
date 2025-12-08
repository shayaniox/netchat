#include <errno.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void err_exit(const char *format, ...)
{
    va_list arg;
    va_start(arg, format);
    fprintf(stderr, "\x1b[31m[ERROR]\x1b[0m ");
    vfprintf(stderr, format, arg);
    fprintf(stderr, " %s", errno != 0 ? strerror(errno) : "");
    fprintf(stderr, "\n");
    va_end(arg);
    exit(EXIT_SUCCESS);
}
