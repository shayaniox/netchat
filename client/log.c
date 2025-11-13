#include <fcntl.h>
#include <stdarg.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

int log_file(const char *format, ...)
{
    int fd = open("./program.log", O_RDWR | O_CREAT | O_APPEND, 0644);
    if (fd == -1)
        return -1;

    time_t t = time(NULL);
    struct tm time_info;
    localtime_r(&t, &time_info);

    char timebuf[64];
    strftime(timebuf, sizeof(timebuf), "[%T]", &time_info);

    if (write(fd, timebuf, strlen(timebuf)) == -1) {
        close(fd);
        return -1;
    }

    va_list args;
    va_start(args, format);
    vdprintf(fd, format, args);
    va_end(args);

    close(fd);
    return 0;
}
