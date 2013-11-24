#include <defs.h>
#include <stdlib.h>

char buf[10] = "FORK";

int main(int argc, char* argv[])
{
    pid_t pid;
    printf("\nProcess %d (parent %d) says %s", getpid(), getppid(), buf);

    if ((pid = fork()) == 0) {
        printf("\nFork return: %d Process %d (parent %d): child", pid, getpid(), getppid());
    } else {
        printf("\nFork return: %d Process %d (parent %d): parent", pid, getpid(), getppid());
    }

    while(1);
    return 0;
}

