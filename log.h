#ifndef _LOG_H
#define _LOG_H

#include <stdio.h>

#define info(M, ...) printf("\x1b[32m[INFO]\x1b[0m " M "\n", ##__VA_ARGS__);
#define error(M, ...) printf("\x1b[31m[ERROR]\x1b[0m " M "\n", ##__VA_ARGS__);

#endif
