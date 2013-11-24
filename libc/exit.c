#include <syscall.h>

void exit(int status)
{
    __syscall0(EXIT);
}

