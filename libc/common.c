#include <defs.h>
#include <syscall.h>

pid_t getpid()
{
    return __syscall0(GETPID);
}

pid_t getppid()
{
    return __syscall0(GETPPID);
}

pid_t fork()
{
    return __syscall0(FORK);
}

