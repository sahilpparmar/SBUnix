#include <defs.h>
#include <syscall.h>
#include <stdlib.h>
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

pid_t wait(uint64_t *status)
{
    return __syscall1(WAIT, (uint64_t)status);
}

pid_t waitpid(pid_t pid, uint64_t *status, int options)
{
    return __syscall3(WAITPID, (uint64_t)pid, (uint64_t)status, (uint64_t)options);
}

void listprocess()
{
    __syscall0(LISTPROCESS);
}

void yield()
{
    __syscall0(YIELD);
}
