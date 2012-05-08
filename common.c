
#include "common.h"

int die(const char *fmt, ...)
{
    va_list args;
    va_start(args, fmt);
    int n = vfprintf(stderr, fmt, args);
    va_end(args);
    return n;
}
