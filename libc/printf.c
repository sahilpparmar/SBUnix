#include <syscall.h>

int printf(const char *format, ...)
{
    return __syscall1(PUTS, (uint64_t) format);
}

