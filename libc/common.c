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

int execvpe(const char *file, char *const argv[], char *const envp[])
{
    return __syscall3(EXECVPE, (uint64_t)file, (uint64_t)argv, (uint64_t)envp);
}

void listprocess()
{
    __syscall0(LISTPROCESS);
}

