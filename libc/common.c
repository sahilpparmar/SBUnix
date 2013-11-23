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

void listprocess()
{
    __syscall0(LISTPROCESS);
}
